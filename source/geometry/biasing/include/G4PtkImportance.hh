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
// $Id: G4PtkImportance.hh,v 1.5 2002/04/10 13:13:07 dressel Exp $
// GEANT4 tag $Name: geant4-04-01 $
//
// ----------------------------------------------------------------------
// Class G4PtkImportance
//
// Class description:
//
// Used internally by importance sampling. 
// It is acontainer for "cell" importance value pairs.

// Author: Michael Dressel (Michael.Dressel@cern.ch)
// ----------------------------------------------------------------------
#ifndef G4PtkImportance_hh
#define G4PtkImportance_hh G4PtkImportance_hh 

#include "g4std/map"
#include "globals.hh"
#include "G4PTouchableKey.hh"

typedef G4std::map<G4PTouchableKey, G4double, G4PTkComp> G4PtkImportance;
  // implement container G4PtkImportance as map

G4std::ostream& operator<<(G4std::ostream &out, const G4PtkImportance &ptki);

#endif