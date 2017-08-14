/*
 * Copyright (C) 2016-  Qiming Sun <osirpt.sun@gmail.com>
 * Description: code generated by  gen-code.cl
 */
#include "grid_ao_drv.h"
/*  #C(0 1) G |GTO> */
static void shell_eval_GTOval_ig(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*2];
double fy0[SIMDD*16*2];
double fz0[SIMDD*16*2];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double buf[SIMDD*nc*3];
double s[SIMDD*3];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double c[3];
c[0] = 1 * (-ri[0]);
c[1] = 1 * (-ri[1]);
c[2] = 1 * (-ri[2]);

for (j = 0; j < 3; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+1; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R0I(1, 0, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*c[1]*s[2*SIMDD+n]) + c[2]*s[1*SIMDD+n];
buf[1*SIMDD+n] = + (-1*c[2]*s[0*SIMDD+n]) + c[0]*s[2*SIMDD+n];
buf[2*SIMDD+n] = + (-1*c[0]*s[1*SIMDD+n]) + c[1]*s[0*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+1; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R0I(1, 0, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*c[1]*s[2*SIMDD+n]) + c[2]*s[1*SIMDD+n];
buf[1*SIMDD+n] = + (-1*c[2]*s[0*SIMDD+n]) + c[0]*s[2*SIMDD+n];
buf[2*SIMDD+n] = + (-1*c[0]*s[1*SIMDD+n]) + c[1]*s[0*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_ig_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 3};
GTOeval_cart_drv(shell_eval_GTOval_ig, GTOprim_exp, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ig_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 3};
GTOeval_sph_drv(shell_eval_GTOval_ig, GTOprim_exp, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ig_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 3};
GTOeval_spinor_drv(shell_eval_GTOval_ig, GTOprim_exp, CINTc2s_iket_spinor_sf1, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
/*  #C(0 1) NABLA G |GTO> */
static void shell_eval_GTOval_ipig(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*4];
double fy0[SIMDD*16*4];
double fz0[SIMDD*16*4];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double *fx2 = fx1 + SIMDD*16;
double *fy2 = fy1 + SIMDD*16;
double *fz2 = fz1 + SIMDD*16;
double *fx3 = fx2 + SIMDD*16;
double *fy3 = fy2 + SIMDD*16;
double *fz3 = fz2 + SIMDD*16;
double buf[SIMDD*nc*9];
double s[SIMDD*9];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double *gto3 = cgto + nao*ngrids*3;
double *gto4 = cgto + nao*ngrids*4;
double *gto5 = cgto + nao*ngrids*5;
double *gto6 = cgto + nao*ngrids*6;
double *gto7 = cgto + nao*ngrids*7;
double *gto8 = cgto + nao*ngrids*8;
double c[3];
c[0] = 1 * (-ri[0]);
c[1] = 1 * (-ri[1]);
c[2] = 1 * (-ri[2]);

for (j = 0; j < 9; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R0I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_R0I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*c[1]*s[2*SIMDD+n]) + c[2]*s[1*SIMDD+n];
buf[1*SIMDD+n] = + (-1*c[2]*s[0*SIMDD+n]) + c[0]*s[2*SIMDD+n];
buf[2*SIMDD+n] = + (-1*c[0]*s[1*SIMDD+n]) + c[1]*s[0*SIMDD+n];
buf[3*SIMDD+n] = + (-1*c[1]*s[5*SIMDD+n]) + c[2]*s[4*SIMDD+n];
buf[4*SIMDD+n] = + (-1*c[2]*s[3*SIMDD+n]) + c[0]*s[5*SIMDD+n];
buf[5*SIMDD+n] = + (-1*c[0]*s[4*SIMDD+n]) + c[1]*s[3*SIMDD+n];
buf[6*SIMDD+n] = + (-1*c[1]*s[8*SIMDD+n]) + c[2]*s[7*SIMDD+n];
buf[7*SIMDD+n] = + (-1*c[2]*s[6*SIMDD+n]) + c[0]*s[8*SIMDD+n];
buf[8*SIMDD+n] = + (-1*c[0]*s[7*SIMDD+n]) + c[1]*s[6*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R0I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_R0I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*c[1]*s[2*SIMDD+n]) + c[2]*s[1*SIMDD+n];
buf[1*SIMDD+n] = + (-1*c[2]*s[0*SIMDD+n]) + c[0]*s[2*SIMDD+n];
buf[2*SIMDD+n] = + (-1*c[0]*s[1*SIMDD+n]) + c[1]*s[0*SIMDD+n];
buf[3*SIMDD+n] = + (-1*c[1]*s[5*SIMDD+n]) + c[2]*s[4*SIMDD+n];
buf[4*SIMDD+n] = + (-1*c[2]*s[3*SIMDD+n]) + c[0]*s[5*SIMDD+n];
buf[5*SIMDD+n] = + (-1*c[0]*s[4*SIMDD+n]) + c[1]*s[3*SIMDD+n];
buf[6*SIMDD+n] = + (-1*c[1]*s[8*SIMDD+n]) + c[2]*s[7*SIMDD+n];
buf[7*SIMDD+n] = + (-1*c[2]*s[6*SIMDD+n]) + c[0]*s[8*SIMDD+n];
buf[8*SIMDD+n] = + (-1*c[0]*s[7*SIMDD+n]) + c[1]*s[6*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_ipig_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_cart_drv(shell_eval_GTOval_ipig, GTOprim_exp, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipig_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_sph_drv(shell_eval_GTOval_ipig, GTOprim_exp, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipig_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_spinor_drv(shell_eval_GTOval_ipig, GTOprim_exp, CINTc2s_iket_spinor_sf1, 0.5,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
/*  SIGMA DOT P |GTO> */
static void shell_eval_GTOval_sp(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*2];
double fy0[SIMDD*16*2];
double fz0[SIMDD*16*2];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double buf[SIMDD*nc*4];
double s[SIMDD*3];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double *gto3 = cgto + nao*ngrids*3;

for (j = 0; j < 4; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+1; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_D_I(1, 0, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*s[0*SIMDD+n]);
buf[1*SIMDD+n] = + (-1*s[1*SIMDD+n]);
buf[2*SIMDD+n] = + (-1*s[2*SIMDD+n]);
buf[3*SIMDD+n] = 0;
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+1; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_D_I(1, 0, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*s[0*SIMDD+n]);
buf[1*SIMDD+n] = + (-1*s[1*SIMDD+n]);
buf[2*SIMDD+n] = + (-1*s[2*SIMDD+n]);
buf[3*SIMDD+n] = 0;
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_sp_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 1};
GTOeval_cart_drv(shell_eval_GTOval_sp, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_sp_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 1};
GTOeval_sph_drv(shell_eval_GTOval_sp, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_sp_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 1};
GTOeval_spinor_drv(shell_eval_GTOval_sp, GTOprim_exp, CINTc2s_ket_spinor_si1, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
/*  NABLA SIGMA DOT P |GTO> */
static void shell_eval_GTOval_ipsp(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*4];
double fy0[SIMDD*16*4];
double fz0[SIMDD*16*4];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double *fx2 = fx1 + SIMDD*16;
double *fy2 = fy1 + SIMDD*16;
double *fz2 = fz1 + SIMDD*16;
double *fx3 = fx2 + SIMDD*16;
double *fy3 = fy2 + SIMDD*16;
double *fz3 = fz2 + SIMDD*16;
double buf[SIMDD*nc*12];
double s[SIMDD*9];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double *gto3 = cgto + nao*ngrids*3;
double *gto4 = cgto + nao*ngrids*4;
double *gto5 = cgto + nao*ngrids*5;
double *gto6 = cgto + nao*ngrids*6;
double *gto7 = cgto + nao*ngrids*7;
double *gto8 = cgto + nao*ngrids*8;
double *gto9 = cgto + nao*ngrids*9;
double *gto10 = cgto + nao*ngrids*10;
double *gto11 = cgto + nao*ngrids*11;

for (j = 0; j < 12; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_D_I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_D_I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*s[0*SIMDD+n]);
buf[1*SIMDD+n] = + (-1*s[1*SIMDD+n]);
buf[2*SIMDD+n] = + (-1*s[2*SIMDD+n]);
buf[3*SIMDD+n] = 0;
buf[4*SIMDD+n] = + (-1*s[3*SIMDD+n]);
buf[5*SIMDD+n] = + (-1*s[4*SIMDD+n]);
buf[6*SIMDD+n] = + (-1*s[5*SIMDD+n]);
buf[7*SIMDD+n] = 0;
buf[8*SIMDD+n] = + (-1*s[6*SIMDD+n]);
buf[9*SIMDD+n] = + (-1*s[7*SIMDD+n]);
buf[10*SIMDD+n] = + (-1*s[8*SIMDD+n]);
buf[11*SIMDD+n] = 0;
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
gto9[j1*ngrids+i+n] += buf[9*SIMDD+n] * coeff[j*np+k];
gto10[j1*ngrids+i+n] += buf[10*SIMDD+n] * coeff[j*np+k];
gto11[j1*ngrids+i+n] += buf[11*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_D_I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_D_I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + (-1*s[0*SIMDD+n]);
buf[1*SIMDD+n] = + (-1*s[1*SIMDD+n]);
buf[2*SIMDD+n] = + (-1*s[2*SIMDD+n]);
buf[3*SIMDD+n] = 0;
buf[4*SIMDD+n] = + (-1*s[3*SIMDD+n]);
buf[5*SIMDD+n] = + (-1*s[4*SIMDD+n]);
buf[6*SIMDD+n] = + (-1*s[5*SIMDD+n]);
buf[7*SIMDD+n] = 0;
buf[8*SIMDD+n] = + (-1*s[6*SIMDD+n]);
buf[9*SIMDD+n] = + (-1*s[7*SIMDD+n]);
buf[10*SIMDD+n] = + (-1*s[8*SIMDD+n]);
buf[11*SIMDD+n] = 0;
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
gto9[j1*ngrids+i+n] += buf[9*SIMDD+n] * coeff[j*np+k];
gto10[j1*ngrids+i+n] += buf[10*SIMDD+n] * coeff[j*np+k];
gto11[j1*ngrids+i+n] += buf[11*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_ipsp_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 3};
GTOeval_cart_drv(shell_eval_GTOval_ipsp, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipsp_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 3};
GTOeval_sph_drv(shell_eval_GTOval_ipsp, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipsp_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {4, 3};
GTOeval_spinor_drv(shell_eval_GTOval_ipsp, GTOprim_exp, CINTc2s_ket_spinor_si1, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
/*  NABLA RC |GTO> */
static void shell_eval_GTOval_iprc(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*4];
double fy0[SIMDD*16*4];
double fz0[SIMDD*16*4];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double *fx2 = fx1 + SIMDD*16;
double *fy2 = fy1 + SIMDD*16;
double *fz2 = fz1 + SIMDD*16;
double *fx3 = fx2 + SIMDD*16;
double *fy3 = fy2 + SIMDD*16;
double *fz3 = fz2 + SIMDD*16;
double buf[SIMDD*nc*9];
double s[SIMDD*9];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double *gto3 = cgto + nao*ngrids*3;
double *gto4 = cgto + nao*ngrids*4;
double *gto5 = cgto + nao*ngrids*5;
double *gto6 = cgto + nao*ngrids*6;
double *gto7 = cgto + nao*ngrids*7;
double *gto8 = cgto + nao*ngrids*8;
double dri[3];
dri[0] = ri[0] - env[PTR_COMMON_ORIG+0];
dri[1] = ri[1] - env[PTR_COMMON_ORIG+1];
dri[2] = ri[2] - env[PTR_COMMON_ORIG+2];

for (j = 0; j < 9; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_RCI(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_RCI(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + s[0*SIMDD+n];
buf[1*SIMDD+n] = + s[1*SIMDD+n];
buf[2*SIMDD+n] = + s[2*SIMDD+n];
buf[3*SIMDD+n] = + s[3*SIMDD+n];
buf[4*SIMDD+n] = + s[4*SIMDD+n];
buf[5*SIMDD+n] = + s[5*SIMDD+n];
buf[6*SIMDD+n] = + s[6*SIMDD+n];
buf[7*SIMDD+n] = + s[7*SIMDD+n];
buf[8*SIMDD+n] = + s[8*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_RCI(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_RCI(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + s[0*SIMDD+n];
buf[1*SIMDD+n] = + s[1*SIMDD+n];
buf[2*SIMDD+n] = + s[2*SIMDD+n];
buf[3*SIMDD+n] = + s[3*SIMDD+n];
buf[4*SIMDD+n] = + s[4*SIMDD+n];
buf[5*SIMDD+n] = + s[5*SIMDD+n];
buf[6*SIMDD+n] = + s[6*SIMDD+n];
buf[7*SIMDD+n] = + s[7*SIMDD+n];
buf[8*SIMDD+n] = + s[8*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_iprc_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_cart_drv(shell_eval_GTOval_iprc, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_iprc_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_sph_drv(shell_eval_GTOval_iprc, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_iprc_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_spinor_drv(shell_eval_GTOval_iprc, GTOprim_exp, CINTc2s_ket_spinor_sf1, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
/*  NABLA R |GTO> */
static void shell_eval_GTOval_ipr(double *cgto, double *ri, double *exps,
double *coord, double *alpha, double *coeff, double *env,
int l, int np, int nc, int nao, int ngrids, int bgrids)
{
const int degen = (l+1)*(l+2)/2;
int lx, ly, lz, i, j, j1, k, l1, n;
double e;
double *pgto;
double *gridx = coord;
double *gridy = coord+BLKSIZE;
double *gridz = coord+BLKSIZE*2;
double fx0[SIMDD*16*4];
double fy0[SIMDD*16*4];
double fz0[SIMDD*16*4];
double *fx1 = fx0 + SIMDD*16;
double *fy1 = fy0 + SIMDD*16;
double *fz1 = fz0 + SIMDD*16;
double *fx2 = fx1 + SIMDD*16;
double *fy2 = fy1 + SIMDD*16;
double *fz2 = fz1 + SIMDD*16;
double *fx3 = fx2 + SIMDD*16;
double *fy3 = fy2 + SIMDD*16;
double *fz3 = fz2 + SIMDD*16;
double buf[SIMDD*nc*9];
double s[SIMDD*9];
double *gto0 = cgto;
double *gto1 = cgto + nao*ngrids*1;
double *gto2 = cgto + nao*ngrids*2;
double *gto3 = cgto + nao*ngrids*3;
double *gto4 = cgto + nao*ngrids*4;
double *gto5 = cgto + nao*ngrids*5;
double *gto6 = cgto + nao*ngrids*6;
double *gto7 = cgto + nao*ngrids*7;
double *gto8 = cgto + nao*ngrids*8;

for (j = 0; j < 9; j++) {
        pgto = cgto + j*nao*ngrids;
        for (n = 0; n < degen*nc; n++) {
        for (i = 0; i < bgrids; i++) {
                pgto[n*ngrids+i] = 0;
        } }
}
for (i = 0; i < bgrids+1-SIMDD; i+=SIMDD) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, SIMDD)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R_I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_R_I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + s[0*SIMDD+n];
buf[1*SIMDD+n] = + s[1*SIMDD+n];
buf[2*SIMDD+n] = + s[2*SIMDD+n];
buf[3*SIMDD+n] = + s[3*SIMDD+n];
buf[4*SIMDD+n] = + s[4*SIMDD+n];
buf[5*SIMDD+n] = + s[5*SIMDD+n];
buf[6*SIMDD+n] = + s[6*SIMDD+n];
buf[7*SIMDD+n] = + s[7*SIMDD+n];
buf[8*SIMDD+n] = + s[8*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < SIMDD; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }

if (i < bgrids) {
        for (k = 0; k < np; k++) {
                if (_nonzero_in(exps+k*BLKSIZE+i, bgrids-i)) {
for (n = 0; n < SIMDD; n++) {
        fx0[n] = 1;
        fy0[n] = 1;
        fz0[n] = 1;
}
for (lx = 1; lx <= l+2; lx++) {
for (n = 0; n < SIMDD; n++) {
        fx0[lx*SIMDD+n] = fx0[(lx-1)*SIMDD+n] * gridx[i+n];
        fy0[lx*SIMDD+n] = fy0[(lx-1)*SIMDD+n] * gridy[i+n];
        fz0[lx*SIMDD+n] = fz0[(lx-1)*SIMDD+n] * gridz[i+n];
} }
GTO_R_I(1, 0, l+0);
GTO_D_I(2, 0, l+1);
GTO_R_I(3, 2, l+0);
for (lx = l, l1 = 0; lx >= 0; lx--) {
        for (ly = l - lx; ly >= 0; ly--, l1++) {
                lz = l - lx - ly;
                for (n = 0; n < SIMDD; n++) {
                       e = exps[k*BLKSIZE+i+n];
s[0*SIMDD+n] = e * fx3[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[1*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[2*SIMDD+n] = e * fx2[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[3*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[4*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy3[ly*SIMDD+n] * fz0[lz*SIMDD+n];
s[5*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy2[ly*SIMDD+n] * fz1[lz*SIMDD+n];
s[6*SIMDD+n] = e * fx1[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[7*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy1[ly*SIMDD+n] * fz2[lz*SIMDD+n];
s[8*SIMDD+n] = e * fx0[lx*SIMDD+n] * fy0[ly*SIMDD+n] * fz3[lz*SIMDD+n];
                }
                for (n = 0; n < SIMDD; n++) {
buf[0*SIMDD+n] = + s[0*SIMDD+n];
buf[1*SIMDD+n] = + s[1*SIMDD+n];
buf[2*SIMDD+n] = + s[2*SIMDD+n];
buf[3*SIMDD+n] = + s[3*SIMDD+n];
buf[4*SIMDD+n] = + s[4*SIMDD+n];
buf[5*SIMDD+n] = + s[5*SIMDD+n];
buf[6*SIMDD+n] = + s[6*SIMDD+n];
buf[7*SIMDD+n] = + s[7*SIMDD+n];
buf[8*SIMDD+n] = + s[8*SIMDD+n];
                }
                for (j = 0, j1 = l1; j < nc; j++, j1+=degen) {
                for (n = 0; n < bgrids-i; n++) {
gto0[j1*ngrids+i+n] += buf[0*SIMDD+n] * coeff[j*np+k];
gto1[j1*ngrids+i+n] += buf[1*SIMDD+n] * coeff[j*np+k];
gto2[j1*ngrids+i+n] += buf[2*SIMDD+n] * coeff[j*np+k];
gto3[j1*ngrids+i+n] += buf[3*SIMDD+n] * coeff[j*np+k];
gto4[j1*ngrids+i+n] += buf[4*SIMDD+n] * coeff[j*np+k];
gto5[j1*ngrids+i+n] += buf[5*SIMDD+n] * coeff[j*np+k];
gto6[j1*ngrids+i+n] += buf[6*SIMDD+n] * coeff[j*np+k];
gto7[j1*ngrids+i+n] += buf[7*SIMDD+n] * coeff[j*np+k];
gto8[j1*ngrids+i+n] += buf[8*SIMDD+n] * coeff[j*np+k];
} } } } } } }
}
void GTOval_ipr_cart(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_cart_drv(shell_eval_GTOval_ipr, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipr_sph(int ngrids, int *shls_slice, int *ao_loc,
double *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_sph_drv(shell_eval_GTOval_ipr, GTOprim_exp, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table,
atm, natm, bas, nbas, env);
}
void GTOval_ipr_spinor(int ngrids, int *shls_slice, int *ao_loc,
double complex *ao, double *coord, char *non0table,
int *atm, int natm, int *bas, int nbas, double *env)
{
int param[] = {1, 9};
GTOeval_spinor_drv(shell_eval_GTOval_ipr, GTOprim_exp, CINTc2s_ket_spinor_sf1, 1,
ngrids, param, shls_slice, ao_loc, ao, coord, non0table, atm, natm, bas, nbas, env);
}
