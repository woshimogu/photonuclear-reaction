//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "B1DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::B1DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1DetectorConstruction::~B1DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* B1DetectorConstruction::Construct()
{  
  // Get nist material manager
    G4NistManager* manager = G4NistManager::Instance();
    //ore_mat
    G4double density, fractionmass;
    G4int ncomponents;
    G4Element* elFe = manager->FindOrBuildElement(26); //Fe
    G4Element* elO = manager->FindOrBuildElement(8); //Na
    G4Element* elSi = manager->FindOrBuildElement(14); // Cs
    G4Element* elMg = manager->FindOrBuildElement(12); //Ba
    G4Element* elCa = manager->FindOrBuildElement(20); //Gd
    G4Element* elAl = manager->FindOrBuildElement(13);
    G4Element* elU = manager->FindOrBuildElement(92);
    density = 1.67*g/cm3;
    G4Material* ore_mat = new G4Material("Ore", density, ncomponents = 7);
    ore_mat->AddElement(elFe, fractionmass= 32.1 * perCent);
    ore_mat->AddElement(elO, fractionmass= 30.1 * perCent);
    ore_mat->AddElement(elSi, fractionmass= 15.1 * perCent);
    ore_mat->AddElement(elMg, fractionmass= 13.9 * perCent);
    ore_mat->AddElement(elCa, fractionmass= 1.5 * perCent);
    ore_mat->AddElement(elAl, fractionmass= 1.4 * perCent);
    ore_mat->AddElement(elU, fractionmass= 0.3 * perCent);



    G4Material* vaccum = manager->FindOrBuildMaterial("G4_Galactic");
    //G4Material* ore_mat = manager->FindOrBuildMaterial("G4_Fe");
  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //     
  // World
  //
  G4double world_sizeXY = 0.5 * meter;
  G4double world_sizeZ  = 0.5 * meter;

  G4Box* solidWorld =    
    new G4Box("World",                       //its name
       0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);     //its size
      
  G4LogicalVolume* logicWorld =                         
    new G4LogicalVolume(solidWorld,          //its solid
                        vaccum,           //its material
                        "World");            //its name
                                   
  G4VPhysicalVolume* physWorld = 
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking
                     
  //     
  // Envelope
  //
  G4double ore_sizeXY = 10.*cm;
  G4double ore_sizeZ = 5.5*cm;
  G4Box* solidOre =
    new G4Box("Ore",                    //its name
        0.5*ore_sizeXY, 0.5*ore_sizeXY, 0.5*ore_sizeZ); //its size
      
  G4LogicalVolume* logicOre =
    new G4LogicalVolume(solidOre,            //its solid
                        ore_mat,             //its material
                        "Ore");         //its name
               
  new G4PVPlacement(0,                       //no rotation
                    G4ThreeVector(),         //at (0,0,0)
                    logicOre,                //its logical volume
                    "Ore",              //its name
                    logicWorld,              //its mother  volume
                    false,                   //no boolean operation
                    0,                       //copy number
                    checkOverlaps);          //overlaps checking

  // Set Shape2 as scoring volume
  //
  //fScoringVolume = logicOre;
  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
