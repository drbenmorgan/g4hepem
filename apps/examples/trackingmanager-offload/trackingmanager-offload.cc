//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file trackingmanager-offload.cc
//---------------------------------------------------------------------------//

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <FTFP_BERT.hh>
#include <G4Box.hh>
#include <G4Electron.hh>
#include <G4EmParameters.hh>
#include <G4EmStandardPhysics.hh>
#include <G4Gamma.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <G4Positron.hh>
#include <G4RunManagerFactory.hh>
#include <G4NistManager.hh>
#include <G4VUserActionInitialization.hh>
#include <G4HadronicProcessStore.hh>
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4SystemOfUnits.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4TrackingManager.hh>
#include <G4UImanager.hh>
//---------------------------------------------------------------------------//
#include "G4HepEmTrackingManager.hh"

//---------------------------------------------------------------------------//
// - Custom PhysicsConstructor to add the Offloader's G4VTrackingManager to
//   e-/e+/gamma particles.
// - Inherited from G4EmStandardPhysics because we want to ensure that the
//   CPU processes/models are constructed properly (To be reviewed, and see
//   also G4 RE07 example).
class EMPhysicsConstructor final : public G4EmStandardPhysics
{
public:
    using G4EmStandardPhysics::G4EmStandardPhysics;

    void ConstructProcess() override
    {
        G4EmStandardPhysics::ConstructProcess();

        auto gpu_tracking = new G4HepEmTrackingManager;
        G4Electron::Definition()->SetTrackingManager(gpu_tracking);
        G4Positron::Definition()->SetTrackingManager(gpu_tracking);
        G4Gamma::Definition()->SetTrackingManager(gpu_tracking);
    }
};

//---------------------------------------------------------------------------//
// Non-coupled to offloader as yet
//---------------------------------------------------------------------------//
class DetectorConstruction final : public G4VUserDetectorConstruction
{
public:
    G4VPhysicalVolume *Construct() final
    {
        auto *box = new G4Box("world", 1000 * cm, 1000 * cm, 1000 * cm);
        auto *lv = new G4LogicalVolume(box, G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb"), "world");
        auto *pv = new G4PVPlacement(
            0, G4ThreeVector{}, lv, "world", nullptr, false, 0);
        return pv;
    }
};

//---------------------------------------------------------------------------//
class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction()
    {
        auto g4particle_def = G4ParticleTable::GetParticleTable()->FindParticle(11);
        gun_.SetParticleDefinition(g4particle_def);
        gun_.SetParticleEnergy(10 * MeV);
        gun_.SetParticlePosition(G4ThreeVector{0, 0, 0});          // origin
        gun_.SetParticleMomentumDirection(G4ThreeVector{1, 0, 0}); // +x
    }

    void GeneratePrimaries(G4Event *event) final
    {
        gun_.GeneratePrimaryVertex(event);
    }

private:
    G4ParticleGun gun_;
};

//---------------------------------------------------------------------------//
// - Set up the needed Run/Event Actions
// - Notify the offloader of the Build/BuildForMaster states
class ActionInitialization final : public G4VUserActionInitialization
{
public:
    void BuildForMaster() const final
    {
    }
    void Build() const final
    {
        this->SetUserAction(new PrimaryGeneratorAction{});
    }
};

//---------------------------------------------------------------------------//

int main()
{
    std::unique_ptr<G4RunManager> run_manager{
        G4RunManagerFactory::CreateRunManager()};
    run_manager->SetUserInitialization(new DetectorConstruction{});

    // Use FTFP_BERT, but replace EM constructor with our own with G4HepEM TM
    auto physics_list = new FTFP_BERT{0};
    physics_list->ReplacePhysics(new EMPhysicsConstructor);
    // Quieten down physics so we can see what's going on
    G4EmParameters::Instance()->SetVerbose(0);
    G4HadronicProcessStore::Instance()->SetVerbose(0);
   
    run_manager->SetUserInitialization(physics_list);
    run_manager->SetUserInitialization(new ActionInitialization());
    run_manager->Initialize();
    run_manager->BeamOn(1);

    return 0;
}
