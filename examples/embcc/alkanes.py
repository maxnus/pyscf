import sys
import logging
import argparse

import numpy as np
from mpi4py import MPI

import pyscf
import pyscf.gto
import pyscf.scf
from pyscf import molstructures
from pyscf import embcc

from util import run_benchmarks

MPI_comm = MPI.COMM_WORLD
MPI_rank = MPI_comm.Get_rank()
MPI_size = MPI_comm.Get_size()

log = logging.getLogger(__name__)

parser = argparse.ArgumentParser(allow_abbrev=False)
parser.add_argument("--basis", default="cc-pVDZ")
parser.add_argument("--solver", default="CCSD")
parser.add_argument("--benchmarks", nargs="*")
parser.add_argument("--c-list", nargs="*", default=list(range(2, 21)))
#parser.add_argument("--c-list", type=int, nargs="*", default=list(range(1, 2)))
parser.add_argument("--local-type", default="IAO")
parser.add_argument("--bath-type", default="mp2-natorb")
parser.add_argument("--bath-energy-tol", type=float, default=1e-10)
parser.add_argument("--density-fit", action="store_true")
parser.add_argument("--max-memory", type=int)
parser.add_argument("--fragment-size", type=int, default=0)
#parser.add_argument("--bath-energy-tol", type=float, default=-1)

parser.add_argument("-o", "--output", default="energies.txt")
args, restargs = parser.parse_known_args()
sys.argv[1:] = restargs

if MPI_rank == 0:
    log.info("Parameters")
    log.info("----------")
    for name, value in sorted(vars(args).items()):
        log.info("%10s: %r", name, value)

for i, n in enumerate(args.c_list):

    if MPI_rank == 0:
        log.info("Number of Carbon atoms: %2d", n)
        log.info("==========================")

    mol = molstructures.build_alkane(n, basis=args.basis, verbose=4, max_memory=args.max_memory)
    nelec = mol.nelectron
    factor = 1.0/mol.nelectron

    mf = pyscf.scf.RHF(mol)
    if args.density_fit:
        #mf = mf.density_fit()
        mf = mf.density_fit(auxbasis="cc-pVDZ-ri")


    t0 = MPI.Wtime()
    mf.kernel()
    mo_stab = mf.stability()[0]
    stable = np.allclose(mo_stab, mf.mo_coeff)
    log.info("Time for HF (s): %.3f", (MPI.Wtime()-t0))
    assert stable
    assert mf.converged

    if args.benchmarks:
        #run_benchmarks(mf, args.benchmarks, n, "benchmarks.txt", print_header=(i==0), factor=factor)
        run_benchmarks(mf, args.benchmarks, nelec, "benchmarks.txt", print_header=(i==0), factor=factor)
        continue

    cc = embcc.EmbCC(mf,
            local_type=args.local_type,
            bath_type=args.bath_type,
            bath_energy_tol=args.bath_energy_tol,
            solver=args.solver,
            )

    solver_opts = {}
    if args.fragment_size == 0:
        cc.make_all_atom_clusters(solver_options=solver_opts)
    elif args.fragment_size == 1:
        cc.make_atom_cluster(["C0", "H1", "H2", "H3"], solver_options=solver_opts)
        k = 4
        for j in range(1, n-1):
            cc.make_atom_cluster(["C%d" % k, "H%d" % (k+1), "H%d" % (k+2)], solver_options=solver_opts)
            k += 3
        cc.make_atom_cluster(["C%d" % k, "H%d" % (k+1), "H%d" % (k+2), "H%d" % (k+3)], solver_options=solver_opts)

    cc.run()

    nactive = [c.nactive for c in cc.clusters]
    #print("Active orbitals: %r" % nactive)
    nactive = np.amax(nactive)
    norb = mol.nao_nr()
    #print("Max active orbitals: %d" % nactive)

    if MPI_rank == 0:
        if (i == 0):
            with open(args.output, "a") as f:
                f.write("#IRC  N  M  HF  EmbCC  dMP2  EmbCC+dMP2\n")
        with open(args.output, "a") as f:
            #f.write(("%2d" + 5*"  %12.8e" + "\n") % (n, mf.e_tot, cc.e_tot, cc.e_delta_mp2, cc.e_tot+cc.e_delta_mp2, mf.e_tot+cc.e_corr_full))
            #f.write(("%2d  " + 5*"  %16.12e" + "\n") % (nelec, factor*mf.e_tot, factor*cc.e_tot, factor*cc.e_delta_mp2, factor*(cc.e_tot+cc.e_delta_mp2), factor*(mf.e_tot+cc.e_corr_full)))
            f.write(("%2d  %2d  %2d" + 4*"  %16.12e" + "\n") % (nelec, nactive, norb, factor*mf.e_tot, factor*cc.e_corr, factor*cc.e_delta_mp2, factor*(cc.e_corr+cc.e_delta_mp2)))
