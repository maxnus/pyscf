# Standard libaries
import logging
from collections import OrderedDict

# External libaries
import numpy as np
import scipy
import scipy.linalg
from mpi4py import MPI

# Internal libaries
import pyscf
import pyscf.lo
import pyscf.cc
import pyscf.ci
import pyscf.mcscf
import pyscf.fci
import pyscf.mp
import pyscf.pbc
import pyscf.pbc.cc
import pyscf.pbc.mp
import pyscf.pbc.tools

# Local modules
from .util import *
from .bath import *
from .energy import *
#from .embcc import VALID_SOLVERS

__all__ = [
        "Cluster",
        ]

MPI_comm = MPI.COMM_WORLD
MPI_rank = MPI_comm.Get_rank()
MPI_size = MPI_comm.Get_size()

log = logging.getLogger(__name__)

class Cluster:

    TOL_CLUSTER_OCC = 1e-3
    TOL_OCC = 1e-3

    def __init__(self, base, cluster_id, name,
            #indices, C_local, C_env,
            C_local, C_env,
            ao_indices=None,
            solver="CCSD",
            bath_type="mp2-natorb", bath_size=None, bath_tol=1e-4,
            **kwargs):
        """
        Parameters
        ----------
        base : EmbCC
            Base EmbCC object.
        cluster_id : int
            Unique ID of cluster.
        name :
            Name of cluster.
        indices:
            Atomic orbital indices of cluster. [ local_orbital_type == "ao" ]
            Intrinsic atomic orbital indices of cluster. [ local_orbital_type == "iao" ]
        """


        self.base = base
        self.id = cluster_id
        self.name = name
        log.debug("Making cluster %d: %s with local orbital type %s", self.id, self.name, self.local_orbital_type)
        #self.indices = indices
        self.ao_indices = ao_indices

        # NEW: local and environment orbitals
        if C_local.shape[-1] == 0:
            raise ValueError("No local orbitals in cluster %d:%s" % (self.id, self.name))

        self.C_local = C_local
        self.C_env = C_env


        # Mean-field electrons
        S = self.mf.get_ovlp()
        dm = np.linalg.multi_dot((self.C_local.T, S, self.mf.make_rdm1(), S, self.C_local))
        ne = np.trace(dm)
        log.info("Mean-field electrons=%.5f", ne)

        #assert self.nlocal == len(self.indices)
        #assert (self.size == len(self.indices) or self.local_orbital_type in ("NonOrth-IAO", "PMO"))

        # Options

        if solver not in self.base.VALID_SOLVERS:
            raise ValueError("Unknown solver: %s" % solver)
        self.solver = solver

        if not hasattr(bath_tol, "__getitem__"):
            bath_tol = (bath_tol, bath_tol)
        if not hasattr(bath_size, "__getitem__"):
            bath_size = (bath_size, bath_size)

        #assert bath_type in (None, "full", "power", "matsubara", "mp2-natorb")
        if bath_type not in self.base.VALID_BATH_TYPES:
            raise ValueError("Unknown bath type: %s" % bath_type)

        self.bath_type = bath_type
        self.bath_target_size = bath_size
        self.bath_tol = bath_tol

        # Other options from kwargs:
        self.solver_options = kwargs.get("solver_options", {})

        #self.mp2_correction = kwargs.get("mp2_correction", True)
        self.mp2_correction = kwargs.get("mp2_correction", True)
        if self.mp2_correction in (True, False):
            self.mp2_correction = (self.mp2_correction, self.mp2_correction)

        # Currently not in use (always on!)
        self.use_ref_orbitals_dmet = kwargs.get("use_ref_orbitals_dmet", True)

        #self.use_ref_orbitals_bath = kwargs.get("use_ref_orbitals_bath", True)
        self.use_ref_orbitals_bath = kwargs.get("use_ref_orbitals_bath", False)

        self.symmetry_factor = kwargs.get("symmetry_factor", 1.0)

        # Restart solver from previous solution [True/False]
        #self.restart_solver = kwargs.get("restart_solver", True)
        self.restart_solver = kwargs.get("restart_solver", False)
        # Parameters needed for restart (C0, C1, C2 for CISD; T1, T2 for CCSD) are saved here
        self.restart_params = kwargs.get("restart_params", {})


        self.dmet_bath_tol = kwargs.get("dmet_bath_tol", 1e-8)

        self.coupled_bath = kwargs.get("coupled_bath", False)

        self.nelectron_target = kwargs.get("nelectron_target", None)


        #self.project_type = kwargs.get("project_type", "first-occ")

        # Orbital coefficents added to this dictionary can be written as an orbital file
        #self.orbitals = {"Fragment" : self.C_local}
        self.orbitals = OrderedDict()
        self.orbitals["Fragment"] = self.C_local

        #
        #self.occbath_eigref = kwargs.get("occbath-eigref", None)
        #self.virbath_eigref = kwargs.get("virbath-eigref", None)
        self.refdata_in = kwargs.get("refdata", {})


        # Maximum number of iterations for consistency of amplitudes between clusters
        self.maxiter = kwargs.get("maxiter", 1)

        self.set_default_attributes()

    def set_default_attributes(self):
        """Set default attributes of cluster object."""

        # Orbitals [set when running solver]
        self.C_bath = None      # DMET bath orbitals
        self.C_occclst = None   # Occupied cluster orbitals
        self.C_virclst = None   # Virtual cluster orbitals
        self.C_occbath = None   # Occupied bath orbitals
        self.C_virbath = None   # Virtual bath orbitals
        #self.nfrozen = None
        self.e_delta_mp2 = 0.0

        self.active = None
        self.active_occ = None
        self.active_vir = None
        self.frozen = None
        self.frozen_occ = None
        self.frozen_vir = None

        # These are used by the solver
        self.mo_coeff = None
        self.mo_occ = None
        self.eris = None

        #self.ref_orbitals = None

        # Reference orbitals should be saved with keys
        # dmet-bath, occ-bath, vir-bath
        #self.refdata = {}
        self.refdata_out = {}
        #self.ref_orbitals = {}

        # Local amplitudes [C1, C2] can be stored here and
        # used by other fragments
        self.amplitudes = {}

        # Orbitals sizes
        #self.nbath0 = 0
        #self.nbath = 0
        #self.nfrozen = 0
        # Calculation results

        self.iteration = 0

        self.converged = False
        self.e_corr = 0.0
        self.e_corr_dmp2 = 0.0

        # For testing:
        # Full cluster correlation energy
        self.e_corr_full = 0.0
        self.e_corr_v = 0.0
        self.e_corr_d = 0.0

        self.e_dmet = 0.0

    #@property
    #def nlocal(self):
    #    """Number of local (fragment) orbitals."""
    #    return self.C_local.shape[-1]

    @property
    def size(self):
        """Number of local (fragment) orbitals."""
        return self.C_local.shape[-1]

    @property
    def ndmetbath(self):
        """Number of DMET bath orbitals."""
        return self.C_bath.shape[-1]

    @property
    def noccbath(self):
        """Number of occupied bath orbitals."""
        return self.C_occbath.shape[-1]

    @property
    def nvirbath(self):
        """Number of virtual bath orbitals."""
        return self.C_virbath.shape[-1]

    @property
    def nactive(self):
        """Number of active orbitals."""
        return len(self.active)

    @property
    def nfrozen(self):
        """Number of frozen environment orbitals."""
        return len(self.frozen)

    #def get_orbitals(self):
    #    """Get dictionary with orbital coefficients."""
    #    orbitals = {
    #            "local" : self.C_local,
    #            "dmet-bath" : self.C_bath,
    #            "occ-bath" : self.C_occbath,
    #            "vir-bath" : self.C_virbath,
    #            }
    #    return orbitals

    def get_refdata(self):
        """Get data of reference calculation for smooth PES."""
        refdata = self.refdata_out
        log.debug("Getting refdata: %r", refdata.keys())
        #refdata = {
        #        "dmet-bath" : self.C_bath,
        #        "occbath-eigref" : self.occbath_eigref,
        #        "virbath-eigref" : self.virbath_eigref,
        #        }
        return refdata

    def set_refdata(self, refdata):
        log.debug("Setting refdata: %r", refdata.keys())
        #self.refdata = refdata
        self.refdata_in = refdata
        #self.dmetbath_ref = refdata["dmet-bath"]
        #self.occbath_eigref = refdata["occbath-eigref"]
        #self.virbath_eigref = refdata["virbath-eigref"]

    def reset(self, keep_ref_orbitals=True):
        """Reset cluster object. By default it stores the previous orbitals, so they can be used
        as reference orbitals for a new calculation of different geometry."""
        ref_orbitals = self.get_orbitals()
        self.set_default_attributes()
        if keep_ref_orbitals:
            self.ref_orbitals = ref_orbitals

    def loop_clusters(self, exclude_self=False):
        """Loop over all clusters."""
        for cluster in self.base.clusters:
            if (exclude_self and cluster == self):
                continue
            yield cluster

    @property
    def mf(self):
        """The underlying mean-field object is taken from self.base.
        This is used throughout the construction of orbital spaces and as the reference for
        the correlated solver.

        Accessed attributes and methods are:
        mf.get_ovlp()
        mf.get_hcore()
        mf.get_fock()
        mf.make_rdm1()
        mf.mo_energy
        mf.mo_coeff
        mf.mo_occ
        mf.e_tot
        """
        return self.base.mf

    @property
    def mol(self):
        """The molecule or cell object is taken from self.base.mol.
        It should be the same as self.base.mf.mol by default."""
        return self.base.mol

    @property
    def has_pbc(self):
        return isinstance(self.mol, pyscf.pbc.gto.Cell)

    @property
    def local_orbital_type(self):
        return self.base.local_orbital_type

    #@property
    #def not_indices(self):
    #    """Indices which are NOT in the cluster, i.e. complement to self.indices."""
    #    return np.asarray([i for i in np.arange(self.mol.nao_nr()) if i not in self.indices])

    @property
    def energy_factor(self):
        return self.symmetry_factor

    # Register frunctions of bath.py as methods
    project_ref_orbitals = project_ref_orbitals
    make_dmet_bath = make_dmet_bath
    make_bath = make_bath
    make_mf_bath = make_mf_bath
    make_mp2_bath = make_mp2_bath
    get_mp2_correction = get_mp2_correction
    transform_mp2_eris = transform_mp2_eris
    run_mp2 = run_mp2

    # Register frunctions of energy.py as methods
    get_local_amplitudes = get_local_amplitudes
    get_local_energy = get_local_energy

    def analyze_orbitals(self, orbitals=None, sort=True):
        if self.local_orbital_type == "iao":
            raise NotImplementedError()


        if orbitals is None:
            orbitals = self.orbitals

        active_spaces = ["local", "dmet-bath", "occ-bath", "vir-bath"]
        frozen_spaces = ["occ-env", "vir-env"]
        spaces = [active_spaces, frozen_spaces, *active_spaces, *frozen_spaces]
        chis = np.zeros((self.mol.nao_nr(), len(spaces)))

        # Calculate chi
        for ao in range(self.mol.nao_nr()):
            S = self.mf.get_ovlp()
            S121 = 1/S[ao,ao] * np.outer(S[:,ao], S[:,ao])
            for ispace, space in enumerate(spaces):
                C = orbitals.get_coeff(space)
                SC = np.dot(S121, C)
                chi = np.sum(SC[ao]**2)
                chis[ao,ispace] = chi

        ao_labels = np.asarray(self.mol.ao_labels(None))
        if sort:
            sort = np.argsort(-np.around(chis[:,0], 3), kind="mergesort")
            ao_labels = ao_labels[sort]
            chis2 = chis[sort]
        else:
            chis2 = chis

        # Output
        log.info("Orbitals of cluster %s", self.name)
        log.info("===================="+len(self.name)*"=")
        log.info(("%18s" + " " + len(spaces)*"  %9s"), "Atomic orbital", "Active", "Frozen", "Local", "DMET bath", "Occ. bath", "Vir. bath", "Occ. env.", "Vir. env")
        log.info((18*"-" + " " + len(spaces)*("  "+(9*"-"))))
        for ao in range(self.mol.nao_nr()):
            line = "[%3s %3s %2s %-5s]:" % tuple(ao_labels[ao])
            for ispace, space in enumerate(spaces):
                line += "  %9.3g" % chis2[ao,ispace]
            log.info(line)

        # Active chis
        return chis[:,0]


    def diagonalize_cluster_dm(self, C_bath):
        """Diagonalize cluster DM to get fully occupied/virtual orbitals

        Parameters
        ----------
        C_bath : ndarray
            DMET bath orbitals.

        Returns
        -------
        C_occclst : ndarray
            Occupied cluster orbitals.
        C_virclst : ndarray
            Virtual cluster orbitals.
        """
        S = self.mf.get_ovlp()
        C_clst = np.hstack((self.C_local, C_bath))
        D_clst = np.linalg.multi_dot((C_clst.T, S, self.mf.make_rdm1(), S, C_clst)) / 2
        e, R = np.linalg.eigh(D_clst)
        tol = self.dmet_bath_tol
        if not np.allclose(np.fmin(abs(e), abs(e-1)), 0, atol=tol, rtol=0):
            raise RuntimeError("Error while diagonalizing cluster DM: eigenvalues not all close to 0 or 1:\n%s", e)
        e, R = e[::-1], R[:,::-1]
        C_clst = np.dot(C_clst, R)
        nocc_clst = sum(e > 0.5)
        nvir_clst = sum(e < 0.5)
        log.info("Number of fragment + DMET-bath orbitals: occ=%3d, vir=%3d", nocc_clst, nvir_clst)

        C_occclst, C_virclst = np.hsplit(C_clst, [nocc_clst])

        return C_occclst, C_virclst

    def canonicalize(self, *C, eigenvalues=False):
        """Diagonalize Fock matrix within subspace.

        Parameters
        ----------
        *C : ndarrays
            Orbital coefficients.

        Returns
        -------
        C : ndarray
            Canonicalized orbital coefficients.
        """
        C = np.hstack(C)
        #F = np.linalg.multi_dot((C.T, self.mf.get_fock(), C))
        F = np.linalg.multi_dot((C.T, self.base.fock, C))
        e, R = np.linalg.eigh(F)
        C = np.dot(C, R)
        if eigenvalues:
            return C, e
        return C

    def get_occup(self, C):
        """Get mean-field occupation numbers (diagonal of 1-RDM) of orbitals.

        Parameters
        ----------
        C : ndarray, shape(N, M)
            Orbital coefficients.

        Returns
        -------
        occ : ndarray, shape(M)
            Occupation numbers of orbitals.
        """
        S = self.mf.get_ovlp()
        D = np.linalg.multi_dot((C.T, S, self.mf.make_rdm1(), S, C))
        occ = np.diag(D)
        return occ

    def get_local_projector(self, C, kind="right", inverse=False):
        """Projector for one index of amplitudes local energy expression.

        Parameters
        ----------
        C : ndarray, shape(N, M)
            Occupied or virtual orbital coefficients.
        kind : str ["right", "left", "center"], optional
            Only for AO local orbitals.
        inverse : bool, optional
            If true, return the environment projector 1-P instead.

        Return
        ------
        P : ndarray, shape(M, M)
            Projection matrix.
        """
        S = self.mf.get_ovlp()


        # Project onto space of local (fragment) orbitals.
        #if self.local_orbital_type in ("IAO", "LAO"):
        if self.local_orbital_type in ("IAO", "LAO", "PMO"):
            CSC = np.linalg.multi_dot((C.T, S, self.C_local))
            P = np.dot(CSC, CSC.T)

        # Project onto space of local atomic orbitals.
        #elif self.local_orbital_type in ("AO", "NonOrth-IAO", "PMO"):
        elif self.local_orbital_type in ("AO", "NonOrth-IAO"):
            #l = self.indices
            l = self.ao_indices
            # This is the "natural way" to truncate in AO basis
            if kind == "right":
                P = np.linalg.multi_dot((C.T, S[:,l], C[l]))
            # These two methods - while exact in the full bath limit - might require some thought...
            # See also: CCSD in AO basis paper of Scuseria et al.
            elif kind == "left":
                P = np.linalg.multi_dot((C[l].T, S[l], C))
            elif kind == "center":
                s = scipy.linalg.fractional_matrix_power(S, 0.5)
                assert np.isclose(np.linalg.norm(s.imag), 0)
                s = s.real
                assert np.allclose(np.dot(s, s), S)
                P = np.linalg.multi_dot((C.T, s[:,l], s[l], C))
            else:
                raise ValueError("Unknown kind=%s" % kind)

        if inverse:
            P = (np.eye(P.shape[-1]) - P)

            # DEBUG
            #CSC = np.linalg.multi_dot((C.T, S, self.C_env))
            #P2 = np.dot(CSC, CSC.T)
            #assert np.allclose(P, P2)


        return P

    def project_amplitudes(self, P, T1, T2, indices_T1=None, indices_T2=None, symmetrize_T2=False):
        """Project full amplitudes to local space.

        Parameters
        ----------
        P : ndarray
            Projector.
        T1 : ndarray
            C1/T1 amplitudes.
        T2 : ndarray
            C2/T2 amplitudes.

        Returns
        -------
        pT1 : ndarray
            Projected C1/T1 amplitudes
        pT2 : ndarray
            Projected C2/T2 amplitudes
        """
        if indices_T1 is None:
            indices_T1 = [0]
        if indices_T2 is None:
            indices_T2 = [0]

        # T1 amplitudes
        assert indices_T1 == [0]
        if T1 is not None:
            pT1 = einsum("xi,ia->xa", P, T1)
        else:
            pT1 = None

        # T2 amplitudes
        if indices_T2 == [0]:
            pT2 = einsum("xi,ijab->xjab", P, T2)
        elif indices_T2 == [1]:
            pT2 = einsum("xj,ijab->ixab", P, T2)
        elif indices_T2 == [0, 1]:
            pT2 = einsum("xi,yj,ijab->xyab", P, P, T2)

        if symmetrize_T2:
            log.debug("Projected T2 symmetry error = %.3g", np.linalg.norm(pT2 - pT2.transpose(1,0,3,2)))
            pT2 = (pT2 + pT2.transpose(1,0,3,2))/2

        return pT1, pT2

    def transform_amplitudes(self, Ro, Rv, T1, T2):
        if T1 is not None:
            T1 = einsum("xi,ya,ia->xy", Ro, Rv, T1)
        else:
            T1 = None
        T2 = einsum("xi,yj,za,wb,ijab->xyzw", Ro, Ro, Rv, Rv, T2)
        return T1, T2


    def run_solver(self, solver=None, mo_coeff=None, mo_occ=None, active=None, frozen=None, eris=None,
            solver_options=None):
        solver = solver or self.solver
        if mo_coeff is None: mo_coeff = self.mo_coeff
        if mo_occ is None: mo_occ = self.mo_occ
        if active is None: active = self.active
        if frozen is None: frozen = self.frozen
        if eris is None: eris = self.eris
        if solver_options is None: solver_options = self.solver_options

        self.iteration += 1
        if self.iteration > 1:
            log.debug("Iteration %d", self.iteration)

        log.debug("Running solver %s for cluster %s on MPI process %d", solver, self.name, MPI_rank)

        if len(active) == 1:
            log.debug("Only one orbital in cluster. No correlation energy.")
            solver = None

        if self.has_pbc:
            log.debug("Cell object found -> using pbc code.")

        if solver is None:
            log.debug("No solver")
            e_corr_full = 0
            e_corr = 0
            converged = True

        # MP2
        # ===
        elif solver == "MP2":
            if self.has_pbc:
                mp2 = pyscf.pbc.mp.MP2(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)
            else:
                mp2 = pyscf.mp.MP2(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)
            solverobj = mp2

            if eris is None:
                t0 = MPI.Wtime()
                eris = mp2.ao2mo()
                log.debug("Time for integral transformation: %s", get_time_string(MPI.Wtime()-t0))
            e_corr_full, t2 = mp2.kernel(eris=eris)
            converged = True
            e_corr_full *= self.energy_factor
            C1, C2 = None, t2

            pC1, pC2 = self.get_local_amplitudes(mp2, C1, C2)
            e_corr = self.get_local_energy(mp2, pC1, pC2, eris=eris)

        # CCSD
        # ====
        elif solver == "CCSD":
            if self.has_pbc:
                cc = pyscf.pbc.cc.CCSD(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)
            else:
                cc = pyscf.cc.CCSD(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)

            solverobj = cc
            self.cc = cc

            # We want to reuse the integral for local energy
            if eris is None:
                t0 = MPI.Wtime()
                eris = cc.ao2mo()
                log.debug("Time for integral transformation: %s", get_time_string(MPI.Wtime()-t0))
            cc.max_cycle = 100

            # Taylored CC in iterations > 1
            #if True:
            if self.base.tccT1 is not None:
                log.debug("Adding tailorfunc for tailored CC.")

                tcc_mix_factor = 1

                # Transform to cluster basis
                act = cc.get_frozen_mask()
                Co = mo_coeff[:,act][:,mo_occ[act]>0]
                Cv = mo_coeff[:,act][:,mo_occ[act]==0]
                Cmfo = self.mf.mo_coeff[:,self.mf.mo_occ>0]
                Cmfv = self.mf.mo_coeff[:,self.mf.mo_occ==0]
                S = self.mf.get_ovlp()
                Ro = np.linalg.multi_dot((Co.T, S, Cmfo))
                Rv = np.linalg.multi_dot((Cv.T, S, Cmfv))

                ttcT1, ttcT2 = self.transform_amplitudes(Ro, Rv, self.base.tccT1, self.base.tccT2)
                #ttcT1 = 0
                #ttcT2 = 0

                # Get occupied bath projector
                #Pbath = self.get_local_projector(Co, inverse=True)
                ##Pbath2 = self.get_local_projector(Co)
                ##log.debug("%r", Pbath)
                ##log.debug("%r", Pbath2)
                ##1/0
                ##CSC = np.linalg.multi_dot((Co.T, S, self.C_env))
                ##Pbath2 = np.dot(CSC, CSC.T)
                ##assert np.allclose(Pbath, Pbath2)

                ##CSC = np.linalg.multi_dot((Co.T, S, np.hstack((self.C_occclst, self.C_occbath))))
                #CSC = np.linalg.multi_dot((Co.T, S, np.hstack((self.C_bath, self.C_occbath))))
                #Pbath2 = np.dot(CSC, CSC.T)
                #assert np.allclose(Pbath, Pbath2)

                #log.debug("DIFF %g", np.linalg.norm(Pbath - Pbath2))
                #log.debug("DIFF %g", np.linalg.norm(Pbath + Pbath2 - np.eye(Pbath.shape[-1])))

                def tailorfunc(T1, T2):
                    # Difference of bath to local amplitudes
                    dT1 = ttcT1 - T1
                    dT2 = ttcT2 - T2

                    # Project difference amplitudes to bath-bath block in occupied indices
                    #pT1, pT2 = self.project_amplitudes(Co, dT1, dT2, indices_T2=[0, 1])
                    ###pT1, pT2 = self.project_amplitudes(Pbath, dT1, dT2, indices_T2=[0, 1])
                    ###_, pT2_0 = self.project_amplitudes(Pbath, None, dT2, indices_T2=[0])
                    ###_, pT2_1 = self.project_amplitudes(Pbath, None, dT2, indices_T2=[1])
                    ###pT2 += (pT2_0 + pT2_1)/2

                    # Inverse=True gives the non-local (bath) part
                    pT1, pT2 = self.get_local_amplitudes(cc, dT1, dT2, inverse=True)

                    #pT12, pT22 = self.get_local_amplitudes(cc, pT1, pT2, inverse=True, symmetrize=False)
                    #assert np.allclose(pT12, pT1)
                    #assert np.allclose(pT22, pT2)
                    #pT1, pT2 = self.get_local_amplitudes(cc, dT1, dT2, variant="democratic", inverse=True)
                    # Subtract to get non-local amplitudes
                    #pT1 = dT1 - pT1
                    #pT2 = dT2 - pT2

                    log.debug("Norm of pT1=%6.2g, dT1=%6.2g, pT2=%6.2g, dT2=%6.2g", np.linalg.norm(dT1), np.linalg.norm(pT1), np.linalg.norm(dT2), np.linalg.norm(pT2))
                    # Add projected difference amplitudes
                    T1 += tcc_mix_factor*pT1
                    T2 += tcc_mix_factor*pT2
                    return T1, T2

                cc.tailorfunc = tailorfunc

            # Use FCI amplitudes
            #if True:
            #if False:
            if self.coupled_bath:
                log.debug("Coupling bath")
                for x in self.loop_clusters(exclude_self=True):
                    if not x.amplitudes:
                        continue
                    log.debug("Coupling bath with fragment %s with solver %s", x.name, x.solver)

                #act = cc.get_frozen_mask()
                #Co = mo_coeff[:,act][:,mo_occ[act]>0]
                #Cv = mo_coeff[:,act][:,mo_occ[act]==0]
                #no = Co.shape[-1]
                #nv = Cv.shape[-1]

                #T1_ext = np.zeros((no, nv))
                #T2_ext = np.zeros((no, no, nv, nv))
                #Pl = []
                #Po = []
                #Pv = []

                #for x in self.loop_clusters(exclude_self=True):
                #    if not x.amplitudes:
                #        continue
                #    log.debug("Amplitudes found in cluster %s with solver %s", x.name, x.solver)
                #    #C_x_occ = x.amplitudes["C_occ"]
                #    #C_x_vir = x.amplitudes["C_vir"]
                #    Cx_occ = x.C_occact
                #    Cx_vir = x.C_viract
                #    C1x = x.amplitudes["C1"]
                #    C2x = x.amplitudes["C2"]

                #    actx = x.cc.get_frozen_mask()
                #    assert np.allclose(Cx_occ, x.cc.mo_coeff[:,actx][x.cc.mo_occ[actx]>0])
                #    assert np.allclose(Cx_vir, x.cc.mo_coeff[:,actx][x.cc.mo_occ[actx]==0])

                #    assert Cx_occ.shape[-1] == C1x.shape[0]
                #    assert Cx_vir.shape[-1] == C1x.shape[1]

                #    T1x, T2x = amplitudes_C2T(C1x, C2x)

                #    # Project to local first index
                #    Plx = x.get_local_projector(Cx_occ)
                #    T1x = einsum("xi,ia->xa", Plx, T1x)
                #    T2x = einsum("xi,ijab->xjab", Plx, T2x)

                #    # Transform to current basis
                #    S = self.mf.get_ovlp()
                #    Rox = np.linalg.multi_dot((Co.T, S, Cx_occ))
                #    Rvx = np.linalg.multi_dot((Cv.T, S, Cx_vir))
                #    T1x, T2x = self.transform_amplitudes(Rox, Rvx, T1x, T2x)

                #    T1_ext += T1x
                #    T2_ext += T2x

                #    Plx = np.linalg.multi_dot((x.C_local.T, S, Co))
                #    Pox = np.linalg.multi_dot((x.C_occclst.T, S, Co))
                #    Pvx = np.linalg.multi_dot((x.C_virclst.T, S, Cv))
                #    Pl.append(Plx)
                #    Po.append(Pox)
                #    Pv.append(Pvx)

                #Pl = np.vstack(Pl)
                #Pl = np.dot(Pl.T, Pl)
                #Po = np.vstack(Po)
                #Po = np.dot(Po.T, Po)
                #Pv = np.vstack(Pv)
                #Pv = np.dot(Pv.T, Pv)

                #def tailorfunc(T1, T2):
                #    # Difference amplitudes
                #    dT1 = (T1_ext - T1)
                #    dT2 = (T2_ext - T2)
                #    # Project to Px
                #    #pT1 = einsum("xi,ia->xa", Pext, dT1)
                #    #pT2 = einsum("xi,ijab->xjab", Pext, dT2)
                #    pT1 = einsum("xi,ya,ia->xy", Pl, Pv, dT1)
                #    pT2 = einsum("xi,yj,za,wb,ijab->xyzw", Pl, Po, Pv, Pv, dT2)
                #    # Symmetrize pT2
                #    pT2 = (pT2 + pT2.transpose(1,0,3,2))/2
                #    T1 += pT1
                #    T2 += pT2
                #    return T1, T2
                S = self.mf.get_ovlp()

                def tailorfunc(T1, T2):
                    T1out = T1.copy()
                    T2out = T2.copy()
                    for x in self.loop_clusters(exclude_self=True):
                        if not x.amplitudes:
                            continue
                        C1x, C2x = x.amplitudes["C1"], x.amplitudes["C2"]
                        T1x, T2x = amplitudes_C2T(C1x, C2x)

                        # Remove double counting
                        # Transform to x basis [note that the sizes of the active orbitals will be changed]
                        Ro = np.linalg.multi_dot((x.C_occact.T, S, self.C_occact))
                        Rv = np.linalg.multi_dot((x.C_viract.T, S, self.C_viract))
                        T1x_dc, T2x_dc = self.transform_amplitudes(Ro, Rv, T1, T2)
                        dT1x = (T1x - T1x_dc)
                        dT2x = (T2x - T2x_dc)

                        Px = x.get_local_projector(x.C_occact)
                        pT1x = einsum("xi,ia->xa", Px, dT1x)
                        pT2x = einsum("xi,ijab->xjab", Px, dT2x)

                        # Transform back and add
                        pT1x, pT2x = self.transform_amplitudes(Ro.T, Rv.T, pT1x, pT2x)
                        T1out += pT1x
                        T2out += pT2x

                    return T1out, T2out

                cc.tailorfunc = tailorfunc

            if self.restart_solver:
                log.debug("Running CCSD starting with parameters for: %r...", self.restart_params.keys())
                cc.kernel(eris=eris, **self.restart_params)
            else:
                log.debug("Running CCSD...")
                cc.kernel(eris=eris)
            log.debug("CCSD done. converged: %r", cc.converged)

            if self.restart_solver:
                self.restart_params["t1"] = cc.t1
                self.restart_params["t2"] = cc.t2

            converged = cc.converged
            e_corr_full = self.energy_factor*cc.e_corr
            #C1 = cc.t1
            #C2 = cc.t2 + einsum('ia,jb->ijab', cc.t1, cc.t1)
            C1, C2 = amplitudes_T2C(cc.t1, cc.t2)

            #e_corr = self.get_local_energy(cc, C1, C2, eris=eris)

            pC1, pC2 = self.get_local_amplitudes(cc, C1, C2)
            e_corr = self.get_local_energy(cc, pC1, pC2, eris=eris)

            # Other energy variants
            if False:
                pC1v, pC2v = self.get_local_amplitudes(cc, C1, C2, variant="first-vir")
                pC1d, pC2d = self.get_local_amplitudes(cc, C1, C2, variant="democratic")
                e_corr_v = self.get_local_energy(cc, pC1v, pC2v, eris=eris)
                e_corr_d = self.get_local_energy(cc, pC1d, pC2d, eris=eris)

            # TESTING: Get global amplitudes:
            #if False:
            #if True:
            if self.maxiter > 1:
                log.debug("Maxiter=%3d, storing amplitudes.", self.maxiter)
                if self.base.T1 is None:
                    No = sum(self.mf.mo_occ > 0)
                    Nv = len(self.mf.mo_occ) - No
                    self.base.T1 = np.zeros((No, Nv))
                    self.base.T2 = np.zeros((No, No, Nv, Nv))

                pT1, pT2 = self.get_local_amplitudes(cc, cc.t1, cc.t2)
                #pT1, pT2 = self.get_local_amplitudes(cc, cc.t1, cc.t2, variant="democratic")

                # Transform to HF MO basis
                act = cc.get_frozen_mask()
                occ = cc.mo_occ[act] > 0
                vir = cc.mo_occ[act] == 0
                S = self.mf.get_ovlp()
                Co = cc.mo_coeff[:,act][:,occ]
                Cv = cc.mo_coeff[:,act][:,vir]
                Cmfo = self.mf.mo_coeff[:,self.mf.mo_occ>0]
                Cmfv = self.mf.mo_coeff[:,self.mf.mo_occ==0]
                Ro = np.linalg.multi_dot((Cmfo.T, S, Co))
                Rv = np.linalg.multi_dot((Cmfv.T, S, Cv))
                pT1, pT2 = self.transform_amplitudes(Ro, Rv, pT1, pT2)

                self.base.T1 += pT1
                self.base.T2 += pT2

        elif solver == "CISD":
            # Currently not maintained
            #raise NotImplementedError()
            if self.has_pbc:
                ci = pyscf.pbc.ci.CISD(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)
            else:
                ci = pyscf.ci.CISD(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)
            solverobj = ci

            # We want to reuse the integral for local energy
            if eris is None:
                t0 = MPI.Wtime()
                eris = ci.ao2mo()
                log.debug("Time for integral transformation: %s", get_time_string(MPI.Wtime()-t0))
            ci.max_cycle = 100

            log.debug("Running CISD...")
            if self.nelectron_target is None:
                ci.kernel(eris=eris)
            else:
                # Buggy, doesn't work
                raise NotImplementedError()

                S = self.mf.get_ovlp()
                px = self.get_local_projector(mo_coeff)
                b = np.linalg.multi_dot((S, self.C_local, self.C_local.T, S))

                h1e = self.mf.get_hcore()
                h1e_func = self.mf.get_hcore

                cptmin = 0.0
                cptmax = +3.0
                #cptmin = -0.5
                #cptmax = +0.5

                ntol = 1e-6

                def electron_error(chempot):
                    nonlocal e_tot, wf

                    ci._scf.get_hcore = lambda *args : h1e - chempot*b
                    #ci._scf.get_hcore = lambda *args : h1e
                    ci.kernel(eris=eris)
                    dm1xx = np.linalg.multi_dot((px.T, ci.make_rdm1(), px))
                    nx = np.trace(dm1xx)
                    nerr = (nx - self.nelectron_target)
                    log.debug("chempot=%16.8g, electrons=%16.8g, error=%16.8g", chempot, nx, nerr)
                    assert ci.converged

                    if abs(nerr) < ntol:
                        log.debug("Electron error |%e| below tolerance of %e", nerr, ntol)
                        raise StopIteration

                    return nerr

                try:
                    scipy.optimize.brentq(electron_error, cptmin, cptmax)
                except StopIteration:
                    pass

                # Reset hcore Hamiltonian
                ci._scf.get_hcore = h1e_func

            log.debug("CISD done. converged: %r", ci.converged)

            converged = ci.converged
            e_corr_full = self.energy_factor*ci.e_corr
            # Intermediate normalization
            C0, C1, C2 = ci.cisdvec_to_amplitudes(ci.ci)
            # Renormalize
            C1 *= 1/C0
            C2 *= 1/C0

            pC1, pC2 = self.get_local_amplitudes(ci, C1, C2)
            e_corr = self.get_local_energy(ci, pC1, pC2, eris=eris)

        #elif solver == "FCI":
        elif solver in ("FCI-spin0", "FCI-spin1"):

            nocc_active = len(self.active_occ)
            casci = pyscf.mcscf.CASCI(self.mf, self.nactive, 2*nocc_active)
            solverobj = casci
            # Solver options
            casci.verbose = 10
            casci.canonicalization = False
            #casci.fix_spin_(ss=0)
            # TEST SPIN
            if solver == "FCI-spin0":
                casci.fcisolver = pyscf.fci.direct_spin0.FCISolver(self.mol)
            casci.fcisolver.conv_tol = 1e-9
            casci.fcisolver.threads = 1
            casci.fcisolver.max_cycle = 400
            #casci.fcisolver.level_shift = 5e-3

            if solver_options:
                spin = solver_options.pop("fix_spin", None)
                if spin is not None:
                    log.debug("Setting fix_spin to %r", spin)
                    casci.fix_spin_(ss=spin)

                for key, value in solver_options.items():
                    log.debug("Setting solver attribute %s to value %r", key, value)
                    setattr(casci.fcisolver, key, value)

            # The sorting of the orbitals above should already have placed the CAS in the correct position

            log.debug("Running FCI...")
            if self.nelectron_target is None:
                e_tot, e_cas, wf, *_ = casci.kernel(mo_coeff=mo_coeff)
            # Chemical potential loop
            else:

                S = self.mf.get_ovlp()
                px = self.get_local_projector(mo_coeff)
                b = np.linalg.multi_dot((S, self.C_local, self.C_local.T, S))

                t = np.linalg.multi_dot((S, mo_coeff, px))
                h1e = casci.get_hcore()
                h1e_func = casci.get_hcore

                cptmin = -4
                cptmax = 0
                #cptmin = -0.5
                #cptmax = +0.5

                ntol = 1e-6
                e_tot = None
                wf = None

                def electron_error(chempot):
                    nonlocal e_tot, wf

                    #casci.get_hcore = lambda *args : h1e - chempot*b
                    casci.get_hcore = lambda *args : h1e - chempot*(S-b)

                    e_tot, e_cas, wf, *_ = casci.kernel(mo_coeff=mo_coeff, ci0=wf)
                    #e_tot, e_cas, wf, *_ = casci.kernel(mo_coeff=mo_coeff)
                    dm1xx = np.linalg.multi_dot((t.T, casci.make_rdm1(), t))
                    nx = np.trace(dm1xx)
                    nerr = (nx - self.nelectron_target)
                    log.debug("chempot=%16.8g, electrons=%16.8g, error=%16.8g", chempot, nx, nerr)
                    assert casci.converged

                    if abs(nerr) < ntol:
                        log.debug("Electron error |%e| below tolerance of %e", nerr, ntol)
                        raise StopIteration

                    return nerr

                try:
                    scipy.optimize.brentq(electron_error, cptmin, cptmax)
                except StopIteration:
                    pass

                # Reset hcore Hamiltonian
                casci.get_hcore = h1e_func

            #assert np.allclose(mo_coeff_casci, mo_coeff)
            #dma, dmb = casci.make_rdm1s()
            #log.debug("Alpha: %r", np.diag(dma))
            #log.debug("Beta: %r", np.diag(dmb))
            log.debug("FCI done. converged: %r", casci.converged)
            #log.debug("Shape of WF: %r", list(wf.shape))
            cisdvec = pyscf.ci.cisd.from_fcivec(wf, self.nactive, 2*nocc_active)
            C0, C1, C2 = pyscf.ci.cisd.cisdvec_to_amplitudes(cisdvec, self.nactive, nocc_active)
            # Intermediate normalization
            log.debug("Weight of reference determinant = %.8e", C0)
            renorm = 1/C0
            C1 *= renorm
            C2 *= renorm

            converged = casci.converged
            e_corr_full = self.energy_factor*(e_tot - self.mf.e_tot)

            # Create fake CISD object
            cisd = pyscf.ci.CISD(self.mf, mo_coeff=mo_coeff, mo_occ=mo_occ, frozen=frozen)

            if eris is None:
                t0 = MPI.Wtime()
                eris = cisd.ao2mo()
                log.debug("Time for integral transformation: %s", get_time_string(MPI.Wtime()-t0))

            pC1, pC2 = self.get_local_amplitudes(cisd, C1, C2)
            e_corr = self.get_local_energy(cisd, pC1, pC2, eris=eris)

        else:
            raise ValueError("Unknown solver: %s" % solver)

        # Store integrals for iterations
        if self.maxiter > 1:
            self.eris = eris

        if solver == "FCI":
            # Store amplitudes
            #S = self.mf.get_ovlp()
            # Rotation from occupied to local+DMET
            #Ro = np.linalg.multi_dot((np.hstack((self.C_local, self.C_bath)).T, S, self.C_occclst))
            #assert Ro.shape[0] == Ro.shape[1]
            # Rotation from virtual to local+DMET
            #Rv = np.linalg.multi_dot((np.hstack((self.C_local, self.C_bath)).T, S, self.C_virclst))
            #assert Rv.shape[0] == Rv.shape[1]
            #rC1, rC2 = self.transform_amplitudes((Ro, Rv, pC1, pC2))
            #log.debug("Transforming C2 amplitudes: %r -> %r", list(pC2.shape), list(rC2.shape))
            #self.amplitudes["C_occ"] = mo_coeff[:,mo_occ>0]
            #self.amplitudes["C_vir"] = mo_coeff[:,mo_occ==0]
            self.amplitudes["C1"] = C1
            self.amplitudes["C2"] = C2

        log.debug("Full cluster correlation energy = %.10g htr", e_corr_full)
        self.e_corr_full = e_corr_full

        self.converged = converged
        if self.e_corr != 0.0:
            log.debug("dEcorr=%.8g", (e_corr-self.e_corr))
        self.e_corr = e_corr


        #self.e_corr_dmp2 = e_corr + self.e_delta_mp2

        #self.e_corr_v = e_corr_v
        #self.e_corr_d = e_corr_d

        # RDM1
        if False:
        #if True and solver:

            #if solver != "FCI":
            if not solver.startswith("FCI"):
                dm1 = solverobj.make_rdm1()
                dm2 = solverobj.make_rdm2()
            else:
                dm1, dm2 = pyscf.mcscf.addons.make_rdm12(solverobj, ao_repr=False)

            px = self.get_local_projector(mo_coeff)

            # DMET energy
            if True:
                dm1x = np.einsum("ai,ij->aj", px, dm1)
                dm2x = np.einsum("ai,ijkl->ajkl", px, dm2)
                h1e = np.einsum('pi,pq,qj->ij', mo_coeff, self.mf.get_hcore(), mo_coeff)
                nmo = mo_coeff.shape[-1]
                eri = pyscf.ao2mo.kernel(self.mol, mo_coeff, compact=False).reshape([nmo]*4)
                self.e_dmet = self.energy_factor*(np.einsum('pq,qp', h1e, dm1x) + np.einsum('pqrs,pqrs', eri, dm2x) / 2)
                log.debug("E(DMET) = %e", self.e_dmet)

                # TEST full energy
                #e_full = self.energy_factor*(np.einsum('pq,qp', h1e, dm1) + np.einsum('pqrs,pqrs', eri, dm2) / 2) + self.mol.energy_nuc()
                #log.debug("E(full) = %16.8g htr, reference = %16.8g htr", e_full, solverobj.e_tot)
                #assert np.isclose(e_full, solverobj.e_tot)


            # Count fragment electrons
            dm1xx = np.einsum("ai,ij,bj->ab", px, dm1, px)
            n = np.trace(dm1)
            nx = np.trace(dm1xx)
            log.info("Number of local/total electrons: %12.8f / %12.8f ", nx, n)

            self.nelectron_corr_x = nx

        return converged, e_corr

    def run(self, solver=None,
            #ref_orbitals=None
            refdata=None,
            ):
        """Construct bath orbitals and run solver.

        Paramters
        ---------
        solver : str
            Method ["MP2", "CISD", "CCSD", "FCI"]
        ref_orbitals : dict
            Dictionary with reference orbitals.

        Returns
        -------
        converged : bool
        """

        solver = solver or self.solver
        # Orbitals from a reference calaculation (e.g. different geometry)
        # Used for recovery of orbitals via active transformation
        refdata = refdata or self.refdata_in
        if False:
            ref_orbitals = ref_orbitals or self.ref_orbitals

            # === Make DMET bath orbital and diagonalize DM in cluster space
            if ref_orbitals.get("dmet-bath", None) is not None:
                assert np.allclose(ref_orbitals["dmet-bath"], self.refdata_in["dmet-bath"])

        t0_bath = MPI.Wtime()

        t0 = MPI.Wtime()
        #C_bath, C_occenv, C_virenv = self.make_dmet_bath(C_ref=ref_orbitals.get("dmet-bath", None))
        C_bath, C_occenv, C_virenv = self.make_dmet_bath(C_ref=refdata.get("dmet-bath", None), tol=self.dmet_bath_tol)
        C_occclst, C_virclst = self.diagonalize_cluster_dm(C_bath)
        log.debug("Time for DMET bath: %s", get_time_string(MPI.Wtime()-t0))

        self.C_bath = C_bath

        # Canonicalize cluster
        if False:
            C_occclst, e = self.canonicalize(C_occclst, eigenvalues=True)
            log.debug("Occupied cluster Fock eigenvalues: %r", e)
            C_virclst, e = self.canonicalize(C_virclst, eigenvalues=True)
            log.debug("Virtual cluster Fock eigenvalues: %r", e)

        self.C_occclst = C_occclst
        self.C_virclst = C_virclst

        # For orbital plotting
        self.orbitals["DMET-bath"] = C_bath
        self.orbitals["Occ.-Cluster"] = C_occclst
        self.orbitals["Vir.-Cluster"] = C_virclst

        self.refdata_out["dmet-bath"] = C_bath

        # === Additional bath orbitals
        if self.use_ref_orbitals_bath:
            #C_occref = ref_orbitals.get("occ-bath", None)
            #C_virref = ref_orbitals.get("vir-bath", None)
            #if C_occref is not None:
            #    assert np.allclose(C_occref, self.refdata_in["occ-bath"])
            #if C_virref is not None:
            #    assert np.allclose(C_virref, self.refdata_in["vir-bath"])
            C_occref = refdata.get("occ-bath", None)
            C_virref = refdata.get("vir-bath", None)
        else:
            C_occref = None
            C_virref = None

        # Reorder
        #occbath_eigref = self.occbath_eigref
        #virbath_eigref = self.virbath_eigref
        occbath_eigref = refdata.get("occbath-eigref", None)
        virbath_eigref = refdata.get("virbath-eigref", None)
        #virbath_eigref = self.virbath_eigref

        # TEST
        #occbath_eigref = None
        #virbath_eigref = None

        t0 = MPI.Wtime()
        C_occbath, C_occenv, e_delta_occ, occbath_eigref = self.make_bath(
                C_occenv, self.bath_type, "occ",
                C_ref=C_occref, eigref=occbath_eigref,
                nbath=self.bath_target_size[0], tol=self.bath_tol[0])
        C_virbath, C_virenv, e_delta_vir, virbath_eigref = self.make_bath(
                C_virenv, self.bath_type, "vir",
                C_ref=C_virref, eigref=virbath_eigref,
                nbath=self.bath_target_size[1], tol=self.bath_tol[1])
        log.debug("Time for %r bath: %s", self.bath_type, get_time_string(MPI.Wtime()-t0))
        self.C_occbath = C_occbath
        self.C_virbath = C_virbath
        self.C_occenv = C_occenv
        self.C_virenv = C_virenv

        # For orbital ploting
        self.orbitals["Occ.-bath"] = C_occbath
        self.orbitals["Vir.-bath"] = C_virbath
        self.orbitals["Occ.-env."] = C_occenv
        self.orbitals["Vir.-env."] = C_virenv

        self.refdata_out["occ-bath"] = C_occbath
        self.refdata_out["vir-bath"] = C_virbath

        # For future reorderings
        #self.occbath_eigref = occbath_eigref
        #self.virbath_eigref = virbath_eigref
        self.refdata_out["occbath-eigref"] = occbath_eigref
        self.refdata_out["virbath-eigref"] = virbath_eigref

        self.e_delta_mp2 = e_delta_occ + e_delta_vir
        log.debug("MP2 correction = %.8g", self.e_delta_mp2)

        # FULL MP2 correction [TESTING]
        #if True:
        if False:
            Co1 = np.hstack((C_occclst, C_occenv))
            Cv1 = np.hstack((C_virclst, C_virenv))
            Co2 = np.hstack((C_occclst, C_occbath))
            Cv2 = np.hstack((C_virclst, C_virbath))
            self.e_delta_mp2 = self.get_mp2_correction(Co1, Cv1, Co2, Cv2)
            log.debug("Full MP2 correction = %.8g", self.e_delta_mp2)

        # === Canonicalize orbitals
        if True:
            t0 = MPI.Wtime()
            C_occact = self.canonicalize(C_occclst, C_occbath)
            C_viract = self.canonicalize(C_virclst, C_virbath)
            log.debug("Time for canonicalization: %s", get_time_string(MPI.Wtime()-t0))
        else:
            C_occact = np.hstack((C_occclst, C_occbath))
            C_viract = np.hstack((C_virclst, C_virbath))

        self.C_occact = C_occact
        self.C_viract = C_viract

        # Combine, important to keep occupied orbitals first
        # Put frozen (occenv, virenv) orbitals to the front and back
        # and active orbitals (occact, viract) in the middle
        Co = np.hstack((C_occenv, C_occact))
        Cv = np.hstack((C_viract, C_virenv))
        mo_coeff = np.hstack((Co, Cv))
        No = Co.shape[-1]
        Nv = Cv.shape[-1]
        # Check occupations
        assert np.allclose(self.get_occup(Co), 2, atol=2*self.dmet_bath_tol), "%r" % self.get_occup(Co)
        assert np.allclose(self.get_occup(Cv), 0, atol=2*self.dmet_bath_tol), "%r" % self.get_occup(Cv)
        mo_occ = np.asarray(No*[2] + Nv*[0])

        frozen_occ = list(range(C_occenv.shape[-1]))
        active_occ = list(range(C_occenv.shape[-1], No))
        active_vir = list(range(No, No+C_viract.shape[-1]))
        frozen_vir = list(range(No+C_viract.shape[-1], No+Nv))
        active = active_occ + active_vir
        frozen = frozen_occ + frozen_vir
        # Run solver
        self.mo_coeff = mo_coeff
        self.mo_occ = mo_occ
        self.active = active
        self.active_occ = active_occ
        self.active_vir = active_vir
        self.frozen = frozen
        self.frozen_occ = frozen_occ
        self.frozen_vir = frozen_vir
        # Orbital numbers
        log.debug("Orbitals")
        log.debug("--------")
        log.debug("Active (occ., vir., all) = %4d  %4d  %4d", C_occact.shape[-1], C_viract.shape[-1], self.nactive)
        log.debug("Frozen (occ., vir., all) = %4d  %4d  %4d", C_occenv.shape[-1], C_virenv.shape[-1], self.nfrozen)

        log.debug("Wall time for bath: %s", get_time_string(MPI.Wtime()-t0_bath))

        # Write Cubegen files
        #if True:
        #if False:
        #    orbitals = {
        #        "F" : self.C_local,
        #        "BD" : self.C_bath,
        #        "BO" : self.C_occbath,
        #        "BV" : self.C_virbath,
        #        "EO" : C_occenv,
        #        "EV" : C_virenv,
        #        }

        #    if False:
        #        for orbkind, C in orbitals.items():
        #            for j in range(C.shape[-1]):
        #                filename = "C%d-%s-%d.cube" % (self.id, orbkind, j)
        #                make_cubegen_file(self.mol, C[:,j], filename)
        #    else:
        #        from pyscf.tools import molden
        #        for orbkind, C in orbitals.items():
        #            with open("C%d-%s.molden" % (self.id, orbkind), "w") as f:
        #                molden.header(self.mol, f)
        #                molden.orbital_coeff(self.mol, f, C)


        #    raise SystemExit()

        t0 = MPI.Wtime()
        converged, e_corr = self.run_solver(solver, mo_coeff, mo_occ, active=active, frozen=frozen)
        log.debug("Wall time for solver: %s", get_time_string(MPI.Wtime()-t0))

        #self.converged = converged
        #self.e_corr = e_corr
        #self.e_corr_dmp2 = e_corr + e_delta_mp2

        return converged, e_corr

    def create_orbital_file(self, filename, filetype="molden"):
        if filetype not in ("cube", "molden"):
            raise ValueError("Unknown file type: %s" % filetype)

        ext = {"molden" : "molden", "cube" : "cube"}
        filename = "%s-c%d.%s" % (filename, self.id, ext[filetype])

        if filetype == "molden":
            from pyscf.tools import molden

            orb_labels = {
                    "Fragment" : "F",
                    "DMET-bath" : "D",
                    "Occ.-Cluster" : "O",
                    "Vir.-Cluster" : "V",
                    "Occ.-Bath" : "P",
                    "Vir.-Bath" : "Q",
                    "Occ.-Env." : "R",
                    "Vir.-Env." : "S",
                    }

            with open(filename, "w") as f:
                molden.header(self.mol, f)
                labels = []
                coeffs = []
                for name, C in self.orbitals.items():
                    labels += C.shape[-1]*[name]
                    coeffs.append(C)
                coeffs = np.hstack(coeffs)
                molden.orbital_coeff(self.mol, f, coeffs, symm=labels)

                #for name, C in self.orbitals.items():
                    #symm = orb_labels.get(name, "?")
                    #symm = C.shape[-1] * [name]
                    #molden.orbital_coeff(self.mol, f, C)
                    #molden.orbital_coeff(self.mol, f, C, symm=symm)
        elif filetype == "cube":
            raise NotImplementedError()
            for orbkind, C in self.orbitals.items():
                for j in range(C.shape[-1]):
                    filename = "C%d-%s-%d.cube" % (self.id, orbkind, j)
                    make_cubegen_file(self.mol, C[:,j], filename)

        #orbitals = {
        #    "F" : self.C_local,
        #    "BD" : self.C_bath,
        #    "BO" : self.C_occbath,
        #    "BV" : self.C_virbath,
        #    "EO" : C_occenv,
        #    "EV" : C_virenv,
        #    }

        #if filetype == "molden":
        #    from pyscf.tools import molden
        #    for orbkind, C in orbitals.items():
        #        with open("C%d-%s.molden" % (self.id, orbkind), "w") as f:
        #            molden.header(self.mol, f)
        #            molden.orbital_coeff(self.mol, f, C)
        #elif filetype == "cube":
        #    for orbkind, C in orbitals.items():
        #        for j in range(C.shape[-1]):
        #            filename = "C%d-%s-%d.cube" % (self.id, orbkind, j)
        #            make_cubegen_file(self.mol, C[:,j], filename)

