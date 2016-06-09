//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
// -------------------------------------------------------------------
//
// GEANT4 Class file
//
//
// File name:     G4IonFluctuation
//
// Author:        Vladimir Ivanchenko
//
// Creation date: 03.01.2002
//
// Modifications:
//
// 28-12-02 add method Dispersion (V.Ivanchenko)
// 07-02-03 change signature (V.Ivanchenko)
// 13-02-03 Add name (V.Ivanchenko)
//
// Class Description:
//
// -------------------------------------------------------------------
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#include "G4IonFluctuations.hh"
#include "Randomize.hh"
#include "G4Poisson.hh"
#include "G4Material.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleDefinition.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4IonFluctuations::G4IonFluctuations(const G4String& nam)
 :G4VEmFluctuationModel(nam),
  minNumberInteractionsBohr(10.0),
  theBohrBeta2(50.0*keV/proton_mass_c2),
  minFraction(0.2)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4IonFluctuations::~G4IonFluctuations()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

void G4IonFluctuations::Initialise(const G4ParticleDefinition* part)
{
  particle       = part;
  particleMass   = part->GetPDGMass();
  G4double q     = part->GetPDGCharge()/eplus;
  chargeSquare   = q*q;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double G4IonFluctuations::SampleFluctuations(const G4Material* material,
                                               const G4DynamicParticle* dp,
                                                     G4double& tmax,
                                                     G4double& length,
                                                     G4double& meanLoss)
{

  if(dp->GetDefinition() != particle) {
    particle       = dp->GetDefinition();
    particleMass   = dp->GetMass();
    charge         = dp->GetCharge();
    chargeSquare   = charge*charge;
  }

  G4double siga = Dispersion(material,dp,tmax,length);
  G4double loss = meanLoss;

  // Gaussian fluctuation
  if (meanLoss >= minNumberInteractionsBohr*tmax) {

    // Increase fluctuations for big fractional energy loss
    if ( meanLoss > minFraction*kineticEnergy ) {
      G4double gam = (kineticEnergy - meanLoss)/particleMass + 1.0;
      G4double b2  = 1.0 - 1.0/(gam*gam);
      G4double x   = b2/beta2;
      G4double x3  = 1.0/(x*x*x);
      siga *= 0.25*(1.0 + x)*(x3 + (1.0/b2 - 0.5)/(1.0/beta2 - 0.5) );
    }
    siga = sqrt(siga);
    do {
     loss = G4RandGauss::shoot(meanLoss,siga);
    } while (loss < 0. || loss > 2.*meanLoss);

  // Poisson fluctuations
  } else {
    G4double navr = meanLoss*meanLoss/siga;
    G4double n    = (G4double)G4Poisson(navr);
    loss = meanLoss*n/navr;
  }

  return loss;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double G4IonFluctuations::Dispersion(
                          const G4Material* material,
                          const G4DynamicParticle* dp,
 				G4double& tmax,
			        G4double& length)
{
  G4double electronDensity = material->GetElectronDensity();
  kineticEnergy  = dp->GetKineticEnergy();
  G4double gam   = kineticEnergy/particleMass + 1.0;
  beta2 = 1.0 - 1.0/(gam*gam);
  G4double siga  = (1.0/beta2 - 0.5) * twopi_mc2_rcl2 * tmax * length
                 * electronDensity * chargeSquare;

  // Low velocity - additional ion charge fluctuations according to
  // Q.Yang et al., NIM B61(1991)149-155.
  G4double zeff  = electronDensity/(material->GetTotNbOfAtomsPerVolume());

  if ( beta2 < 3.0*theBohrBeta2*zeff ) {

    G4double a = CoeffitientA (zeff);
    G4double b = CoeffitientB (material, zeff);
    siga *= (chargeSquare * a + b);
  }

  return siga;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double G4IonFluctuations::CoeffitientA(G4double& zeff)
{
  // The aproximation of energy loss fluctuations
  // Q.Yang et al., NIM B61(1991)149-155.

  // Reduced energy in MeV/AMU
  G4double energy = kineticEnergy * amu_c2/(particleMass*MeV) ;
  static G4double a[96][4] = {
 {-0.3291, -0.8312,  0.2460, -1.0220},
 {-0.5615, -0.5898,  0.5205, -0.7258},
 {-0.5280, -0.4981,  0.5519, -0.5865},
 {-0.5125, -0.4625,  0.5660, -0.5190},
 {-0.5127, -0.8595,  0.5626, -0.8721},
 {-0.5174, -1.1930,  0.5565, -1.1980},
 {-0.5179, -1.1850,  0.5560, -1.2070},
 {-0.5209, -0.9355,  0.5590, -1.0250},
 {-0.5255, -0.7766,  0.5720, -0.9412},

 {-0.5776, -0.6665,  0.6598, -0.8484},
 {-0.6013, -0.6045,  0.7321, -0.7671},
 {-0.5781, -0.5518,  0.7605, -0.6919},
 {-0.5587, -0.4981,  0.7835, -0.6195},
 {-0.5466, -0.4656,  0.7978, -0.5771},
 {-0.5406, -0.4690,  0.8031, -0.5718},
 {-0.5391, -0.5061,  0.8024, -0.5974},
 {-0.5380, -0.6483,  0.7962, -0.6970},
 {-0.5355, -0.7722,  0.7962, -0.7839},
 {-0.5329, -0.7720,  0.7988, -0.7846},

 {-0.5335, -0.7671,  0.7984, -0.7933},
 {-0.5324, -0.7612,  0.7998, -0.8031},
 {-0.5305, -0.7300,  0.8031, -0.7990},
 {-0.5307, -0.7178,  0.8049, -0.8216},
 {-0.5248, -0.6621,  0.8165, -0.7919},
 {-0.5180, -0.6502,  0.8266, -0.7986},
 {-0.5084, -0.6408,  0.8396, -0.8048},
 {-0.4967, -0.6331,  0.8549, -0.8093},
 {-0.4861, -0.6508,  0.8712, -0.8432},
 {-0.4700, -0.6186,  0.8961, -0.8132},

 {-0.4545, -0.5720,  0.9227, -0.7710},
 {-0.4404, -0.5226,  0.9481, -0.7254},
 {-0.4288, -0.4778,  0.9701, -0.6850},
 {-0.4199, -0.4425,  0.9874, -0.6539},
 {-0.4131, -0.4188,  0.9998, -0.6332},
 {-0.4089, -0.4057,  1.0070, -0.6218},
 {-0.4039, -0.3913,  1.0150, -0.6107},
 {-0.3987, -0.3698,  1.0240, -0.5938},
 {-0.3977, -0.3608,  1.0260, -0.5852},
 {-0.3972, -0.3600,  1.0260, -0.5842},

{-0.3985, -0.3803,  1.0200, -0.6013},
 {-0.3985, -0.3979,  1.0150, -0.6168},
 {-0.3968, -0.3990,  1.0160, -0.6195},
 {-0.3971, -0.4432,  1.0050, -0.6591},
 {-0.3944, -0.4665,  1.0010, -0.6825},
 {-0.3924, -0.5109,  0.9921, -0.7235},
 {-0.3882, -0.5158,  0.9947, -0.7343},
 {-0.3838, -0.5125,  0.9999, -0.7370},
 {-0.3786, -0.4976,  1.0090, -0.7310},
 {-0.3741, -0.4738,  1.0200, -0.7155},

 {-0.3969, -0.4496,  1.0320, -0.6982},
 {-0.3663, -0.4297,  1.0430, -0.6828},
 {-0.3630, -0.4120,  1.0530, -0.6689},
 {-0.3597, -0.3964,  1.0620, -0.6564},
 {-0.3555, -0.3809,  1.0720, -0.6454},
 {-0.3525, -0.3607,  1.0820, -0.6289},
 {-0.3505, -0.3465,  1.0900, -0.6171},
 {-0.3397, -0.3570,  1.1020, -0.6384},
 {-0.3314, -0.3552,  1.1130, -0.6441},
 {-0.3235, -0.3531,  1.1230, -0.6498},

 {-0.3150, -0.3483,  1.1360, -0.6539},
 {-0.3060, -0.3441,  1.1490, -0.6593},
 {-0.2968, -0.3396,  1.1630, -0.6649},
 {-0.2935, -0.3225,  1.1760, -0.6527},
 {-0.2797, -0.3262,  1.1940, -0.6722},
 {-0.2704, -0.3202,  1.2100, -0.6770},
 {-0.2815, -0.3227,  1.2480, -0.6775},
 {-0.2880, -0.3245,  1.2810, -0.6801},
 {-0.3034, -0.3263,  1.3270, -0.6778},
 {-0.2936, -0.3215,  1.3430, -0.6835},

 {-0.3282, -0.3200,  1.3980, -0.6650},
 {-0.3260, -0.3070,  1.4090, -0.6552},
 {-0.3511, -0.3074,  1.4470, -0.6442},
 {-0.3501, -0.3064,  1.4500, -0.6442},
 {-0.3490, -0.3027,  1.4550, -0.6418},
 {-0.3487, -0.3048,  1.4570, -0.6447},
 {-0.3478, -0.3074,  1.4600, -0.6483},
 {-0.3501, -0.3283,  1.4540, -0.6669},
 {-0.3494, -0.3373,  1.4550, -0.6765},
 {-0.3485, -0.3373,  1.4570, -0.6774},

 {-0.3462, -0.3300,  1.4630, -0.6728},
 {-0.3462, -0.3225,  1.4690, -0.6662},
 {-0.3453, -0.3094,  1.4790, -0.6553},
 {-0.3844, -0.3134,  1.5240, -0.6412},
 {-0.3848, -0.3018,  1.5310, -0.6303},
 {-0.3862, -0.2955,  1.5360, -0.6237},
 {-0.4262, -0.2991,  1.5860, -0.6115},
 {-0.4278, -0.2910,  1.5900, -0.6029},
 {-0.4303, -0.2817,  1.5940, -0.5927},
 {-0.4315, -0.2719,  1.6010, -0.5829},

 {-0.4359, -0.2914,  1.6050, -0.6010},
 {-0.4365, -0.2982,  1.6080, -0.6080},
 {-0.4253, -0.3037,  1.6120, -0.6150},
 {-0.4335, -0.3245,  1.6160, -0.6377},
 {-0.4307, -0.3292,  1.6210, -0.6447},
 {-0.4284, -0.3204,  1.6290, -0.6380},
 {-0.4227, -0.3217,  1.6360, -0.6438}
  } ;

  G4int iz = (G4int)zeff - 2 ;
  if( 0 > iz ) iz = 0 ;
  if(95 < iz ) iz = 95 ;

  G4double q = 1.0 / (1.0 + a[iz][0]*pow(energy,a[iz][1])+
                          + a[iz][2]*pow(energy,a[iz][3])) ;

  return q ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

G4double G4IonFluctuations::CoeffitientB(const G4Material* material, G4double& zeff)
{
  // The aproximation of energy loss fluctuations
  // Q.Yang et al., NIM B61(1991)149-155.

  // Reduced energy in MeV/AMU
  G4double energy = kineticEnergy *amu_c2/(particleMass*MeV) ;

  G4int i = 0 ;
  G4double factor = 1.0 ;

  // The index of set of parameters i = 0 for protons(hadrons) in gases
  //                                    1 for protons(hadrons) in solids
  //                                    2 for ions in atomic gases
  //                                    3 for ions in molecular gases
  //                                    4 for ions in solids
  static G4double b[5][4] = {
  {0.1014,  0.3700,  0.9642,  3.987},
  {0.1955,  0.6941,  2.522,   1.040},
  {0.05058, 0.08975, 0.1419, 10.80},
  {0.05009, 0.08660, 0.2751,  3.787},
  {0.01273, 0.03458, 0.3951,  3.812}
  } ;

  // protons (hadrons)
  if(1.5 > charge) {
    if( kStateGas != material->GetState() ) i = 1 ;

  // ions
  } else {
    factor = charge * pow(charge/zeff, 0.3333) ;

    if( kStateGas == material->GetState() ) {
      energy /= (charge * sqrt(charge)) ;

      if(1 == (material->GetNumberOfElements())) {
        i = 2 ;
      } else {
        i = 3 ;
      }

    } else {
      energy /= (charge * sqrt(charge*zeff)) ;
      i = 4 ;
    }
  }

  G4double x = b[i][2] * (1.0 - exp( - energy * b[i][3] )) ;

  G4double q = factor * x * b[i][0] /
             ((energy - b[i][1])*(energy - b[i][1]) + x*x) ;

  return q ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....