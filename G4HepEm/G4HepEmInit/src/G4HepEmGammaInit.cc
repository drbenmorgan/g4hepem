
#include "G4HepEmGammaInit.hh"

#include "G4HepEmData.hh"
#include "G4HepEmParameters.hh"
#include "G4HepEmGammaData.hh"

#include "G4HepEmGammaTableBuilder.hh"

// g4 includes
#include "G4Version.hh"

#include "G4EmParameters.hh"
#include "G4ProductionCutsTable.hh"

#include "G4ParticleDefinition.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"

#include "G4DataVector.hh"
#include "G4PairProductionRelModel.hh"
#include "G4KleinNishinaCompton.hh"

#include "G4VCrossSectionDataSet.hh"
#include "G4CrossSectionDataSetRegistry.hh"
#if G4VERSION_NUMBER >= 1100
#include "G4GammaNuclearXS.hh"
#endif
#include "G4PhotoNuclearCrossSection.hh"
#include "G4CrossSectionDataStore.hh"

#include "G4HepEmMaterialData.hh"
#include "G4HepEmElementData.hh"

#include <iostream>

void InitGammaData(struct G4HepEmData* hepEmData, struct G4HepEmParameters* /*hepEmPars*/, int verbose) {
  // clean previous G4HepEmElectronData (if any)
  //
  // create G4Models for gamma
  G4ParticleDefinition* g4PartDef = G4Gamma::Gamma();
  if (verbose > 1) std::cout << "     ---  InitGammaData ... " << std::endl;
  // Min/Max energies of the EM model (same as for the loss-tables)
  G4double emModelEMin = G4EmParameters::Instance()->MinKinEnergy();
  G4double emModelEMax = G4EmParameters::Instance()->MaxKinEnergy();
  //
  // 1. Bethe-Heitler model with Coulomb, screening and LPM correction:
  // --- used on [2mc^2 : E_max]
  G4PairProductionRelModel*  modelPP = new G4PairProductionRelModel();
  modelPP->SetLowEnergyLimit(std::max(emModelEMin, 2.0*CLHEP::electron_mass_c2));
  modelPP->SetHighEnergyLimit(emModelEMax);
  // get cuts for secondary e- (not relevant for gamma models, needed only for the model init)
  const G4DataVector* theElCuts = static_cast<const G4DataVector*>(G4ProductionCutsTable::GetProductionCutsTable()->GetEnergyCutsVector(1));
  modelPP->Initialise(g4PartDef, *theElCuts);
  //
  // 2. The simple Klein-Nishina model for Compton scattering:
  // --- used on [E_min : E_max]
  G4KleinNishinaCompton* modelKN = new G4KleinNishinaCompton();
  modelKN->SetLowEnergyLimit(emModelEMin);
  modelKN->SetHighEnergyLimit(emModelEMax);
  modelKN->Initialise(g4PartDef, *theElCuts);
  //
  // 3. The Gamma-nuclear cross section:
  // --- using the `GammaNuclearXS` as the default in Geant4-11.2.2 G4EmExtraPhysics (the alternative is `PhotoNuclearXS`)
  // The `GammaNuclearXS` is availabel from 10.7 but known by the Registry only from 11.0: use `PhotoNuclearXS` before 11.0
#if G4VERSION_NUMBER >= 1100
  G4VCrossSectionDataSet* xs = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet("GammaNuclearXS");
  if (nullptr == xs) {
    xs = new G4GammaNuclearXS();
  }
#else
  G4VCrossSectionDataSet* xs = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet("PhotoNuclearXS");
  if (nullptr == xs) {
    xs = new G4PhotoNuclearCrossSection();
  }
#endif
  xs->BuildPhysicsTable(*g4PartDef);
  G4CrossSectionDataStore hadGNucXSDataStore;
  hadGNucXSDataStore.AddDataSet(xs);
  //
  // === Use the G4HepEmGammaTableBuilder to build all data tables used at
  //     run time: macroscopic cross section tables and target element
  //     selectors for each models.
  //
  // allocate the GammaData (NOTE: shallow only, BuildLambdaTables will complete
  // the allocation) but cleans the memory of the hepEmData->fTheGammaData
  AllocateGammaData(&(hepEmData->fTheGammaData));
  // build macroscopic cross section data for Conversion and Compton
  if (verbose > 1) std::cout << "     ---  BuildLambdaTables ... " << std::endl;
  BuildLambdaTables(modelPP, modelKN, &hadGNucXSDataStore, hepEmData);
  // build element selectors
  if (verbose > 1) std::cout << "     ---  BuildElementSelectorTables ... " << std::endl;
  BuildElementSelectorTables(modelPP, hepEmData);
  //
  // delete all g4 models
  // NOTE: I don't delete this because something is crashing in G4
//  delete modelPP;
  delete modelKN;
}
