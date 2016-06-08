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
//
// -------------------------------------------------------------------
//
// GEANT4 Class header file
//
//
// File name:     G4ionIonisation
//
// Author:        Vladimir Ivanchenko
// 
// Creation date: 07.05.2002
//
// Modifications: 
//
//
// Class Description: 
//
// This class manages the ionisation process for ions.
// it inherites from G4VContinuousDiscreteProcess via G4VEnergyLoss.
// 

// -------------------------------------------------------------------
//

#ifndef G4ionIonisation_h
#define G4ionIonisation_h 1

#include "G4VEnergyLossSTD.hh"

class G4ParticleDefinition;
class G4Track;
class G4Step;
class G4Material;
class G4VEffectiveChargeModel;

class G4ionIonisation : public G4VEnergyLossSTD
{

public:

  G4ionIonisation(const G4String& name = "ionIoni");

  ~G4ionIonisation();
 
  G4bool IsApplicable(const G4ParticleDefinition& p) 
    {return (p.GetPDGCharge() != 0.0 && p.GetPDGMass() > 10.0*MeV);};

  virtual G4double MinPrimaryEnergy(const G4ParticleDefinition* p,
                                    const G4Material*, G4double cut);

  void PrintInfoDefinition() const;
  // Print out of the class parameters

protected:

  const G4ParticleDefinition* DefineBaseParticle(const G4ParticleDefinition* p);

  virtual G4double GetMeanFreePath(const G4Track& track,
                                         G4double previousStepSize,
                                         G4ForceCondition* condition);

  virtual G4double MaxSecondaryEnergy(const G4DynamicParticle* dynParticle);

private:

  void InitialiseProcess();

  // hide assignment operator 
  G4ionIonisation & operator=(const G4ionIonisation &right);
  G4ionIonisation(const G4ionIonisation&);

  const G4ParticleDefinition* theParticle;
  const G4ParticleDefinition* theBaseParticle;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.... 

inline G4double G4ionIonisation::MinPrimaryEnergy(
          const G4ParticleDefinition* p, const G4Material*, G4double cut)
{
  G4double mass = p->GetPDGMass();
  G4double x = 0.5*cut/electron_mass_c2;
  //  G4double y = electron_mass_c2/mass;
  //  G4double g = x*y + sqrt((1. + x)*(1. + x*y*y));
  G4double g = sqrt(1. + x);
  return mass*(g - 1.0); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.... 

inline G4double G4ionIonisation::MaxSecondaryEnergy(const G4DynamicParticle* dynParticle)
{
  G4double mass  = dynParticle->GetMass();
  G4double gamma = dynParticle->GetKineticEnergy()/mass + 1.0;
  G4double ratio = electron_mass_c2/mass;
  G4double tmax = 2.0*electron_mass_c2*(gamma*gamma - 1.) /
                  (1. + 2.0*gamma*ratio + ratio*ratio);
  return tmax; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....


inline G4double G4ionIonisation::GetMeanFreePath(const G4Track& track,
                                                       G4double step,
                                                       G4ForceCondition* cond)
{
  const G4DynamicParticle* dp = track.GetDynamicParticle();
  G4double mRatio    = proton_mass_c2/dp->GetMass();
  G4double q         = dp->GetCharge()/eplus;
  G4double q_2       = q*q;
  SetMassRatio(mRatio);
  SetReduceFactor(1.0/(q_2*mRatio)); 
  SetChargeSquare(q_2);
  SetChargeSquareRatio(q_2);

  return G4VEnergyLossSTD::GetMeanFreePath(track, step, cond);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#endif