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
/// \file B1SteppingAction.cc
/// \brief Implementation of the B1SteppingAction class

#include "B1SteppingAction.hh"
#include "B1EventAction.hh"
#include "B1DetectorConstruction.hh"
#include "B1Run.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4HadronicProcess.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::B1SteppingAction(B1EventAction* eventAction)
: G4UserSteppingAction(),
  fEventAction(eventAction)

{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B1SteppingAction::~B1SteppingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B1SteppingAction::UserSteppingAction(const G4Step* aStep)
{
    B1Run* run
            = static_cast<B1Run*>(G4RunManager::GetRunManager()->GetNonConstCurrentRun());

    G4int ih;
    // count processes
    //
    const G4StepPoint* endPoint = aStep->GetPostStepPoint();
    G4VProcess* process   =
            const_cast<G4VProcess*>(endPoint->GetProcessDefinedStep());
    run->CountProcesses(process);

    // check that an real interaction occured (eg. not a transportation)
    G4StepStatus stepStatus = endPoint->GetStepStatus();
    G4bool transmit = (stepStatus==fGeomBoundary || stepStatus==fWorldBoundary);
    if (transmit) return;

    //energy-momentum balance initialisation
    //
    const G4StepPoint* prePoint = aStep->GetPreStepPoint();
    G4double Q             = - prePoint->GetKineticEnergy();
    G4ThreeVector Pbalance = - prePoint->GetMomentum();

    //initialisation of the nuclear channel identification
    //
    G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
    G4String partName = particle->GetParticleName();
    G4String nuclearChannel = partName;
    G4HadronicProcess* hproc = dynamic_cast<G4HadronicProcess*>(process);
    const G4Isotope* target = NULL;
    if (hproc)
    {
        target = hproc->GetTargetIsotope();
    }
    G4String targetName = "None";
    if (target)
    {
        targetName = target->GetName();
        nuclearChannel += " + " + targetName + " --> ";
    }
    else
    {
        nuclearChannel += " --> ";
    }
    //nuclearChannel += " + " + targetName + " --> ";

    if (aStep->GetTrack()->GetTrackStatus() == fAlive) {
        G4double energy = endPoint->GetKineticEnergy();
    //    analysis->FillH1(ih,energy);
        //
        G4ThreeVector momentum = endPoint->GetMomentum();
        Q        += energy;
        Pbalance += momentum;
        //
        nuclearChannel += partName + " + ";
    }

    //secondaries
    //
    const std::vector<const G4Track*>* secondary
            = aStep->GetSecondaryInCurrentStep();
    for (size_t lp=0; lp<(*secondary).size(); lp++) {
        particle = (*secondary)[lp]->GetDefinition();
        G4String name   = particle->GetParticleName();
        G4String type   = particle->GetParticleType();
        G4double energy = (*secondary)[lp]->GetKineticEnergy();
        run->ParticleCount(name,energy);
        //energy spectrum
        ih = 0;
        if (particle == G4Gamma::Gamma())       ih = 2;
        else if (particle == G4Electron::Electron()) ih = 3;
        else if (particle == G4Neutron::Neutron())   ih = 4;
        else if (particle == G4Proton::Proton())     ih = 5;
        else if (particle == G4Deuteron::Deuteron()) ih = 6;
        else if (particle == G4Alpha::Alpha())       ih = 7;
        else if (type == "nucleus")                  ih = 8;
        else if (type == "meson")                    ih = 9;
        else if (type == "baryon")                   ih = 10;
        //if (ih > 0) analysis->FillH1(ih,energy);
        //atomic mass
        if (type == "nucleus") {
            G4int A = particle->GetAtomicMass();
            //analysis->FillH1(13, A);
        }
        //energy-momentum balance
        G4ThreeVector momentum = (*secondary)[lp]->GetMomentum();
        Q        += energy;
        Pbalance += momentum;
        //count e- from internal conversion together with gamma
        /*if (particle == G4Electron::Electron())
        {
            particle = G4Gamma::Gamma();
        }*/
        //particle flag
        fParticleFlag[particle]++;
    }

    // nuclear channel
    const G4int kMax = 16;
    const G4String conver[] = {"0","","2 ","3 ","4 ","5 ","6 ","7 ","8 ","9 ",
                               "10 ","11 ","12 ","13 ","14 ","15 ","16 "};
    std::map<G4ParticleDefinition*,G4int>::iterator ip;
    for (ip = fParticleFlag.begin(); ip != fParticleFlag.end(); ip++) {
        particle = ip->first;
        G4String name = particle->GetParticleName();
        G4int nb = ip->second;
        if (nb > kMax) nb = kMax;
        G4String Nb = conver[nb];
        if (particle == G4Gamma::Gamma()) {
            Nb = "N ";
            name = "gamma";
        }
        else if(particle == G4Electron::Electron())
        {
            Nb = "N ";
            name = "e-";
        }
        else
        {
            Nb = conver[nb];
        }

        if (ip != fParticleFlag.begin())
        {
            nuclearChannel += " + ";
        }
        nuclearChannel += Nb + name;
    }

    if((-1 != nuclearChannel.find("U238 -->")) && (-1 == nuclearChannel.find("U237")))
    {
        run->CountGF();
    }

    /*just for debug*/
    //G4cout << "\n nuclear channel: " << nuclearChannel << G4endl;

    /*只统计光核反应道，有别的需求注释掉条件语句即可*/
    if("photonNuclear" == process->GetProcessName())
    {
        run->CountNuclearChannel(nuclearChannel, Q);
    }

    fParticleFlag.clear();

    // kill event after first interaction
    //
    G4RunManager::GetRunManager()->AbortEvent();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
