/***********************************************************************
* 
*               *****   ***    ***
*                  *   *   *  *   *
*                 *     ***    ***
*                *     *   *  *   *
*               *****   ***    ***
*
* A FREE Finite Elements Analysis Program in ANSI C for the Windows & 
* the UNIX OS.
*
* Composed and edited and copyright by 
* Professor Dr.-Ing. Frank Rieg, University of Bayreuth, Germany
*
* eMail: 
* frank.rieg@uni-bayreuth.de
* dr.frank.rieg@t-online.de
* 
* V14.0  January 14, 2009
*
* Z88 should compile and run under any UNIX OS and Windows & GTK+.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
***********************************************************************/ 
/***********************************************************************
* diese Compilerunit umfasst: sqsh88 - Elementsteifigkeitsroutine
*                             qbs88  - Berechnung der Matrix b
* 2.1.2010 Rieg
* 6.2.2011 Rieg - Fehler bei sigr beseitigt (kdflag statt kflag)
***********************************************************************/

/***********************************************************************
* Fuer UNIX
***********************************************************************/
#ifdef FR_UNIX
#include <z88r.h>
#include <stdio.h>    /* fprintf */
#endif

/***********************************************************************
* Fuer Windows
***********************************************************************/
#ifdef FR_WIN
#include <z88r.h>
#include <stdio.h>    /* fprintf */
#endif

/***********************************************************************
* Fuer Windows & GTK+
***********************************************************************/
#ifdef FR_GTKWIN
#include <z88r.h>
#include <stdio.h>
#endif

/***********************************************************************
* Formate
***********************************************************************/
#define NL "\n"

#ifdef FR_XDOUB
#define P10E " %+#10.2lE"
#define P11E " %+#11.3lE"
#endif

#ifdef FR_XQUAD
#define P10E " %+#10.2LE"
#define P11E " %+#11.3LE"
#endif

/***********************************************************************
*  Functions
***********************************************************************/
int qbs88(FR_DOUBLE *det,FR_DOUBLE *r,FR_DOUBLE *s,
         FR_DOUBLE *xbar,FR_INT4 *ktyp);
FR_DOUBLE torgh(FR_DOUBLE sig[]);
FR_DOUBLE sheigh(FR_DOUBLE sig[]);
FR_DOUBLE tornh(FR_DOUBLE sig[]);
FR_DOUBLE sheinh(FR_DOUBLE sig[]);
FR_DOUBLE torsh(FR_DOUBLE sig[]);
FR_DOUBLE sheish(FR_DOUBLE sig[]);

/***********************************************************************
* hier beginnt Function sqsh88
***********************************************************************/
int sqsh88(void)
{
extern FILE *fo3,*fo5;

extern FR_DOUBLEAY smw;
extern FR_DOUBLEAY gmw;
extern FR_DOUBLEAY sigvku;

extern FR_INT4AY jsm;
extern FR_INT4AY koi;
extern FR_INT4AY koffs;

extern FR_DOUBLE ul[];
extern FR_DOUBLE h[];
extern FR_DOUBLE xk[],yk[];
extern FR_DOUBLE b[],xx[],d[];

extern FR_DOUBLE emode,rnuee,qparae;

extern FR_INT4 ktyp,ninto,kdflag,isflag,kc,jpri,ngau,ifnili;

FR_DOUBLE eps[5],sig[5],rv[5];

FR_DOUBLE facesz,facasz,r,s,xs,ys,det,xbar,ax,ay,rmin;
FR_DOUBLE rr,phirad,phideg,sinphi,cosphi;
FR_DOUBLE qsin,qcos,sincos,sigr,sigt,taurt,sigv;

FR_INT4 jp[5];
FR_INT4 i,ist,lx,ly,j,k,jk;

int iret;
  
/*----------------------------------------------------------------------
* Gauss-Legendre Stuetzstellen
*---------------------------------------------------------------------*/
static FR_DOUBLE xg[17]= { 0.,
   0., -.5773502691896, -.7745966692415, -.8611363115941,
   0., +.5773502691896,              0., -.3399810435849,
   0.,              0., +.7745966692415, +.3399810435849,
   0.,              0.,              0., +.8611363115941 };

/*----------------------------------------------------------------------
* Gauss-Legendre Stuetzstellen, fix fuer 2 x 2
*---------------------------------------------------------------------*/
static FR_DOUBLE xgo[5]= { 0.,
-.5773502691896,
+.5773502691896,
+.5773502691896,
-.5773502691896};

static FR_DOUBLE ygo[5]= { 0.,
-.5773502691896,
-.5773502691896,
+.5773502691896,
+.5773502691896};

/*----------------------------------------------------------------------
* natuerliche Koordinaten der Eckknoten
*---------------------------------------------------------------------*/
static FR_DOUBLE rkr[5]= { 0.,
                       -1., 1., 1.,-1. };
static FR_DOUBLE rks[5]= { 0.,
                       -1.,-1., 1., 1. };

/*----------------------------------------------------------------------
* xk und yk umspeichern
*---------------------------------------------------------------------*/
for(i = 1;i <= 8;i++)
  {
  xx[i]  = xk[i];
  xx[8+i]= yk[i];
  }

/*----------------------------------------------------------------------
* Materialkonstanten
*---------------------------------------------------------------------*/
facesz= emode/(1. - rnuee*rnuee);
facasz= emode*(1. - rnuee)/( (1. + rnuee)*(1. - 2*rnuee) );
        
/*----------------------------------------------------------------------
* Elastizitaetsmatrix aufstellen: ebener Spannungszustand
*---------------------------------------------------------------------*/
if (ktyp == 2)
  {
  d[1] = facesz;
  d[5] = facesz * rnuee;
  d[9] = 0.;
  d[2] = d[5];
  d[6] = facesz;
  d[10]= 0.;
  d[3] = 0.;
  d[7] = 0.;
  d[11]= facesz * .5 * (1. - rnuee);
  }
        
/*----------------------------------------------------------------------
* Elastizitaetsmatrix aufstellen: ebener Verzerrungszustand
*---------------------------------------------------------------------*/
if (ktyp == 1)
  {
  d[1] = facasz;
  d[5] = facasz * rnuee / (1. - rnuee);
  d[9] = 0.;
  d[2] = d[5];
  d[6] = facasz;
  d[10]= 0.;
  d[3] = 0.;
  d[7] = 0.;
  d[11]= emode / (2.*(1. + rnuee));
  qparae= 1.;
  }

/*----------------------------------------------------------------------
* Elastizitaetsmatrix aufstellen: axialsymmetrischer Spannungszustand
*---------------------------------------------------------------------*/
if (ktyp == 0)
  {
  d[1] = facasz;
  d[5] = facasz * rnuee / (1. - rnuee);
  d[9] = 0.;
  d[13]= d[5];
  d[2] = d[5];
  d[6] = facasz;
  d[10]= 0.;
  d[14]= d[5];
  d[3] = 0.;
  d[7] = 0.;
  d[11]= emode / (2.*(1. + rnuee));
  d[15]= 0.;
  d[4]= d[5];
  d[8]= d[5];
  d[12]= 0.;
  d[16]= facasz;
  }

/*----------------------------------------------------------------------
* Spannungen in den Gauss-Punkten berechnen
*---------------------------------------------------------------------*/
ist= 3; 
if(ktyp == 0) ist= 4;

if(ninto > 0)
  {

/*======================================================================
* Spannungen in den Gauss-Punkten berechnen, variabel
*=====================================================================*/
  for(lx = 1;lx <= ninto;lx++)                              /* 80 */
    {
    r= xg[(lx-1)*4 + ninto];
    for(ly = 1;ly <= ninto;ly++)                            /* 70 */
      {
      s= xg[(ly-1)*4 + ninto];
      
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Matrix b der partiellen Ableitungen & Formfunktionen holen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      iret= qbs88(&det,&r,&s,&xbar,&ktyp);
      if(iret != 0) return(iret);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Verzerrungen berechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      for(k = 1;k <= ist;k++) 
        {
        eps[k]= 0.;
        for(j = 1;j <= 16;j++)
          { 
          eps[k]= eps[k] + b[(k-1)*16 + j] * ul[j];
          }  
        }    
 
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Spannungen berechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      for(k = 1;k <= ist;k++)
        {
        sig[k]= 0.;
        for(j = 1;j <= ist;j++) 
          {
          sig[k]= sig[k] + d[(k-1)*4 + j] * eps[j];
          }   
        } 

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Integrationspunkte in echte Koordinaten umrechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
      xs= 0.;
      ys= 0.;
 
      for(j = 1;j <= 8;j++)
        {
        xs+= h[j] * xx[   j];
        ys+= h[j] * xx[8 +j];
        }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Spannungen ausschreiben
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Torus */
      if(ktyp == 0)
        {
        if(isflag == 0 && ifnili == 0) 
          {
          fprintf(fo3,NL P10E P10E P11E P11E P11E P11E
          ,xs,ys,sig[1],sig[2],sig[3],sig[4]);
          }

        if(isflag == 1 || isflag == 2 || isflag == 3) 
          {
          if(isflag == 1) sigv= torgh(sig);
          if(isflag == 2) sigv= tornh(sig);
          if(isflag == 3) sigv= torsh(sig);

          ngau++;
          sigvku[ngau]+= sigv;

          if(ifnili == 0)
            {
            fprintf(fo3,NL P10E P10E P11E P11E P11E P11E P11E
            ,xs,ys,sig[1],sig[2],sig[3],sig[4],sigv);
            }
          if(jpri == 1)
            {
            fprintf(fo5,NL P11E P11E P11E,xs,ys,sigvku[ngau]);
            }
          gmw[kc]+= sigv; 
          }
        }                                                  /* e if torus */

/* Scheibe */
      if(ktyp == 2)
        {
        if(isflag == 0)                                    /* keine VglSpa.*/ 
          {
          if(kdflag == 1)                                   /* zus. sigr */
            {
            rr= FR_SQRT(xs*xs + ys*ys);
            if(xs == 0.) xs= 1.e-10;
            phirad= FR_ATAN(ys/xs);
            phideg= phirad*57.29578;

            sinphi= FR_SIN(phirad);
            cosphi= FR_COS(phirad);
            qsin= sinphi*sinphi;
            qcos= cosphi*cosphi;
            sincos= sinphi*cosphi;

            sigr= sig[1]*qcos + sig[2]*qsin + 2.*sig[3]*sincos;
            sigt= sig[1]*qsin + sig[2]*qcos - 2.*sig[3]*sincos;
            taurt=(sig[2]-sig[1])*sincos + sig[3]*(qcos-qsin);

            if(ifnili == 0)
              {
              fprintf(fo3,NL P10E P10E P11E P11E P11E P10E P10E P11E P11E P11E
              ,xs,ys,sig[1],sig[2],sig[3],rr,phideg,sigr,sigt,taurt);
              }
            }                                              /* e if zus. sigr */
          else
            {                                              /* ohne sigr */
            if(ifnili == 0)
              {
              fprintf(fo3,NL P10E P10E P11E P11E P11E
              ,xs,ys,sig[1],sig[2],sig[3]); 
              } 
            }                                              /* e if ohne sigr */
          }                                                /* e if ohne VglS */

        if(isflag == 1 || isflag == 2 || isflag == 3)      /* GEH,NH,SH */
          {
          if(isflag == 1) sigv= sheigh(sig);
          if(isflag == 2) sigv= sheinh(sig);
          if(isflag == 3) sigv= sheish(sig);

          ngau++;
          sigvku[ngau]+= sigv;

          if(kdflag == 1)                                   /* zus. sigr */
            {
            rr= FR_SQRT(xs*xs + ys*ys);
            if(xs == 0.) xs= 1.e-10;
            phirad= FR_ATAN(ys/xs);
            phideg= phirad*57.29578;

            sinphi= FR_SIN(phirad);
            cosphi= FR_COS(phirad);
            qsin= sinphi*sinphi;
            qcos= cosphi*cosphi;
            sincos= sinphi*cosphi;

            sigr= sig[1]*qcos + sig[2]*qsin + 2.*sig[3]*sincos;
            sigt= sig[1]*qsin + sig[2]*qcos - 2.*sig[3]*sincos;
            taurt=(sig[2]-sig[1])*sincos + sig[3]*(qcos-qsin);

            if(ifnili == 0)
              {
              fprintf(fo3,NL P10E P10E P11E P11E P11E P10E P10E P11E P11E P11E P11E
              ,xs,ys,sig[1],sig[2],sig[3],rr,phideg,sigr,sigt,taurt,sigv);
              }
            }                                              /* e if zus. sigr */
          else
            {                                              /* ohne sigr */
            if(ifnili == 0)
              {
              fprintf(fo3,NL P10E P10E P11E P11E P11E P11E
              ,xs,ys,sig[1],sig[2],sig[3],sigv);
              }
            }                                             /* e if ohne sigr */
          if(jpri == 1)
            {
            fprintf(fo5,NL P11E P11E P11E,xs,ys,sigvku[ngau]);
            }
          gmw[kc]+= sigv; 
          }                                                /* e if mit GEH */
        }                                                  /* e if scheibe */
      }                                                    /* e 70 */
    }                                                      /* e 80 */
  gmw[kc]/= ninto*ninto;  /* Mittelwert berechnen */

/*======================================================================
* die Eckpunkte berechnen
*=====================================================================*/
  for(lx = 1;lx <= 4;lx++) 
    {
    r= rkr[lx];
    s= rks[lx];
      
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Matrix b der partiellen Ableitungen & Formfunktionen holen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    iret= qbs88(&det,&r,&s,&xbar,&ktyp);
    if(iret != 0) return(iret);
    
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Integrationspunkte in echte Koordinaten umrechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    xs= 0.;
    ys= 0.;
 
    for(j = 1;j <= 8;j++)
      {
      xs+= h[j] * xx[   j];
      ys+= h[j] * xx[8 +j];
      }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* welcher Knoten ist's wirklich?
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for(j = 1;j <= 4;j++)
      {
      ax   = xk[j] - xs;
      ay   = yk[j] - ys;
      rv[j]= FR_SQRT(ax*ax + ay*ay);
      }

    rmin= 1e88;
    for(j = 1;j <= 4;j++)
      {
      if(rv[j] < rmin) 
        {
        rmin= rv[j];
        jk= j;     
        }
      }
    jp[lx]= jk;
    }

/*======================================================================
* Spannungen in den Gauss-Punkten berechnen, fix fuer Z88O
*=====================================================================*/
  for(lx = 1;lx <= 4;lx++) 
    {
    r= xgo[lx];
    s= ygo[lx];
      
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Matrix b der partiellen Ableitungen & Formfunktionen holen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    iret= qbs88(&det,&r,&s,&xbar,&ktyp);
    if(iret != 0) return(iret);
    
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Dehnungen berechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for(k = 1;k <= ist;k++) 
      {
      eps[k]= 0.;
      for(j = 1;j <= 16;j++)
        { 
        eps[k]= eps[k] + b[(k-1)*16 + j] * ul[j];
        } 
      } 
 
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Spannungen berechnen
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    for(k = 1;k <= ist;k++)
      {
      sig[k]= 0.;
      for(j = 1;j <= ist;j++)
        {
        sig[k]= sig[k] + d[(k-1)*4 + j] * eps[j];
        } 
      } 

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Vergleichsspannungen aufaddieren
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    if (ktyp == 0 && isflag == 1) sigv= torgh (sig);
    if (ktyp == 2 && isflag == 1) sigv= sheigh(sig);
    if (ktyp == 0 && isflag == 2) sigv= tornh (sig);
    if (ktyp == 2 && isflag == 2) sigv= sheinh(sig);
    if (ktyp == 0 && isflag == 3) sigv= torsh (sig);
    if (ktyp == 2 && isflag == 3) sigv= sheish(sig);

    smw[koi[koffs[kc]+jp[lx]-1]]+= sigv;
    jsm[koi[koffs[kc]+jp[lx]-1]]++;
    } /* Ende Z88O */
  }   /* Ende Gausspunkte variabel */  

/*----------------------------------------------------------------------
* Spannungen in den Eckknoten berechnen
*---------------------------------------------------------------------*/
if(ninto == 0)
  {
  for(lx = 1;lx <= 4;lx++)                                 /* 140 */
    {
    r= rkr[lx];
    s= rks[lx];
      
/*======================================================================
* Matrix b der partiellen Ableitungen & Formfunktionen holen
*=====================================================================*/
    iret= qbs88(&det,&r,&s,&xbar,&ktyp);
    if(iret != 0) return(iret);
    
/*======================================================================
* Dehnungen berechnen
*=====================================================================*/
    for(k = 1;k <= ist;k++) 
      {
      eps[k]= 0.;
      for(j = 1;j <= 16;j++)
        {
        eps[k]= eps[k] + b[(k-1)*16 + j] * ul[j];
        }
      }
 
/*======================================================================
* Spannungen berechnen
*=====================================================================*/
    for(k = 1;k <= ist;k++)
      {
      sig[k]= 0.;
      for(j = 1;j <= ist;j++)
        {
        sig[k]= sig[k] + d[(k-1)*4 + j] * eps[j];
        }
      }  

/*======================================================================
* Integrationspunkte in echte Koordinaten umrechnen
*=====================================================================*/
    xs= 0.;
    ys= 0.;
 
    for(j = 1;j <= 8;j++)
      {
      xs+= h[j] * xx[   j];
      ys+= h[j] * xx[8 +j];
      }

/*======================================================================
* Spannungen ausschreiben
*=====================================================================*/
/* Torus */
      if(ktyp == 0 && ifnili == 0)
        {
        fprintf(fo3,"\n" P10E P10E P11E P11E P11E P11E
        ,xs,ys,sig[1],sig[2],sig[3],sig[4]);
        }                                                  /* e if torus */

/* Scheibe */
      if(ktyp == 2)
        {
        if(kdflag == 1)                                     /* zus. sigr */
          {
          rr= FR_SQRT(xs*xs + ys*ys);
          if(xs == 0.) xs= 1.e-10;
          phirad= FR_ATAN(ys/xs);
          phideg= phirad*57.29578;

          sinphi= FR_SIN(phirad);
          cosphi= FR_COS(phirad);
          qsin= sinphi*sinphi;
          qcos= cosphi*cosphi;
          sincos= sinphi*cosphi;

          sigr= sig[1]*qcos + sig[2]*qsin + 2.*sig[3]*sincos;
          sigt= sig[1]*qsin + sig[2]*qcos - 2.*sig[3]*sincos;
          taurt=(sig[2]-sig[1])*sincos + sig[3]*(qcos-qsin);

          if(ifnili == 0)
            {
            fprintf(fo3,NL P10E P10E P11E P11E P11E P10E P10E P11E P11E P11E
            ,xs,ys,sig[1],sig[2],sig[3],rr,phideg,sigr,sigt,taurt);
            }
          }                                                /* e if zus. sigr */
        else
          {                                                /* ohne sigr */
          if(ifnili == 0)
            {
            fprintf(fo3,NL P10E P10E P11E P11E P11E
            ,xs,ys,sig[1],sig[2],sig[3]);
            } 
          }                                                /* e if ohne sigr */
        }                                                  /* e if scheibe */
    }                                                      /* e 140 */
  }                                                        /* e if ninto == 0 */

return(0);
}

/***********************************************************************
* hier beginnt Function qbs88
***********************************************************************/
int qbs88(FR_DOUBLE *det,FR_DOUBLE *r,FR_DOUBLE *s,
         FR_DOUBLE *xbar,FR_INT4 *ktyp)
{
/*---------------------------------------------------------------------
* xx geht rein, unveraendert (ex)
* b  geht raus, neu (ex)
* det geht raus, neu
* r,s gehen rein, unveraendert
* xbar geht raus, neu
* ktyp geht rein, unveraendert
*--------------------------------------------------------------------*/

extern FR_DOUBLE h[];
extern FR_DOUBLE b[],xx[],p[];

FR_DOUBLE xj[5], xji[5];          /* ist 2x2 +1 */

FR_DOUBLE rp,sp,rm,sm,rqm,sqm,r2,s2,dum;
        
FR_INT4 i,j,k,k2;
         
/*----------------------------------------------------------------------
* Klammern der Formfunktionen belegen
*---------------------------------------------------------------------*/
rp= 1. + (*r);
sp= 1. + (*s);
rm= 1. - (*r);
sm= 1. - (*s);
rqm= 1. - (*r)*(*r);
sqm= 1. - (*s)*(*s);
r2= 2. * (*r);
s2= 2. * (*s);

/*----------------------------------------------------------------------
* Formfunktionen
*---------------------------------------------------------------------*/
h[1]= .25 *(rp*sp - rqm*sp - sqm*rp);
h[2]= .25 *(rm*sp - rqm*sp - sqm*rm);
h[3]= .25 *(rm*sm - sqm*rm - rqm*sm);
h[4]= .25 *(rp*sm - rqm*sm - sqm*rp);
h[5]= .5 *rqm*sp;
h[6]= .5 *sqm*rm;
h[7]= .5 *rqm*sm;
h[8]= .5 *sqm*rp;

/*----------------------------------------------------------------------
* Partielle Ableitung der Formfunktionen nach r
*---------------------------------------------------------------------*/
p[1]= .25 *(sp + r2*sp -sqm);
p[2]= .25 *((-sp) + r2*sp + sqm);
p[3]= .25 *((-sm) + sqm + r2*sm);
p[4]= .25 *(sm + r2*sm - sqm);
p[5]= .5 *(-r2)*sp;
p[6]= (-.5 )*sqm;
p[7]= .5 *(-r2)*sm;
p[8]= .5 *sqm;

/*----------------------------------------------------------------------
* Partielle Ableitung der Formfunktionen nach s
*---------------------------------------------------------------------*/
p[9] = .25 *(rp - rqm + s2*rp);
p[10]= .25 *(rm - rqm + s2*rm);
p[11]= .25 *((-rm) + s2*rm + rqm);
p[12]= .25 *((-rp) + rqm + s2*rp);
p[13]= .5 *rqm;
p[14]= .5 *(-s2)*rm;
p[15]= (-.5 )*rqm;
p[16]= .5 *(-s2)*rp;

/*----------------------------------------------------------------------
* Jacobi-Matrix am Punkt (r,s) entwickeln
*---------------------------------------------------------------------*/
for(i = 1;i <= 2;i++)
  {
  for(j = 1;j <= 2;j++)
    {
    dum= 0.;
    for(k = 1;k <= 8;k++)
      {
      dum+= p[(i-1)*8 + k] * xx[(j-1)*8 + k];
      }
    xj[(i-1)*2 + j]= dum;
    }
  }

/*----------------------------------------------------------------------
* Jacobi-Determinante am Punkt (r,s) entwickeln
*---------------------------------------------------------------------*/
(*det)= xj[1] * xj[4] - xj[3] * xj[2];

if((*det) < 0.00000001)
  return(AL_JACNEG);

/*----------------------------------------------------------------------
* Berechnung der inversen Jacobi-Matrix
*---------------------------------------------------------------------*/
dum= 1./(*det);

xji[1]= xj[4] * dum;
xji[2]= (-xj[2]) * dum;
xji[3]= (-xj[3]) * dum;
xji[4]= xj[1] * dum;

/*----------------------------------------------------------------------
* Entwickeln der Matrix b
*---------------------------------------------------------------------*/
for(i = 1;i <= 64;i++)
  b[i]= 0.;

k2= 0;

for(k = 1;k <= 8;k++)
  {
  k2+= 2;
  b[k2-1]= 0.;
  b[k2  ]= 0.;
  b[16 + k2-1]= 0.;
  b[16 + k2  ]= 0.;

  for(i = 1;i <= 2;i++)
    {
    b[     k2-1]= b[     k2-1] + xji[   i] * p[(i-1)*8 + k];
    b[16 + k2  ]= b[16 + k2  ] + xji[2 +i] * p[(i-1)*8 + k];
    }
  b[32 + k2  ]= b[    k2-1];
  b[32 + k2-1]= b[16 +k2  ];
  }

if((*ktyp) > 0) return(0);

/*----------------------------------------------------------------------
* im Falle des axialsymmetrischen Toruselementes 
* die folgende Normalspannungskomponente einfuegen
*---------------------------------------------------------------------*/
/*======================================================================
* Radius am Punkt (r,s) berechnen
*=====================================================================*/
(*xbar)= 0.;

for(k = 1;k <= 8;k++)
  (*xbar)= (*xbar) + h[k] * xx[k];

if((*xbar) <= 0.00000001)
  {
/*======================================================================
* Radius ist null
*=====================================================================*/
  for(k = 1;k <= 16;k++)
    b[48 + k]= b[k];

  return(0);

  }
else
  {
/*======================================================================
* Radius ist nicht null
*=====================================================================*/
  dum=1./(*xbar);
  k2= 0;

  for(k = 1;k <= 8;k++)
    {
    k2+= 2;
    b[48 + k2  ]= 0.;
    b[48 + k2-1]= h[k] * dum;
    }

  }
/**********************************************************************/
return(0);
}
