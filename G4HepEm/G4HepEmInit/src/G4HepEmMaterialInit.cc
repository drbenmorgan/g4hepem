
#include "G4HepEmMaterialInit.hh"

#include "G4HepEmData.hh"
#include "G4HepEmParameters.hh"

#include "G4HepEmMatCutData.hh"
#include "G4HepEmMaterialData.hh"
#include "G4HepEmElementData.hh" 

// g4 includes
#include "G4ProductionCutsTable.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4ElementVector.hh"

#include <vector>
#include <iostream>


// - translates all G4MaterialCutsCouple, used in the current geometry, to a 
//   G4HepEmMatCutData structure element used by G4HepEm
// - generates a G4HepEmMaterialData structure that stores material information 
//   for all unique materials, used in the current geometry
// - builds the G4HepEmElementData structure
void InitMaterialAndCoupleData(struct G4HepEmData* hepEmData, struct G4HepEmParameters* hepEmPars) {
  // get material-cuts couple and material tables
  G4ProductionCutsTable* theCoupleTable = G4ProductionCutsTable::GetProductionCutsTable();
  G4MaterialTable*     theMaterialTable = G4Material::GetMaterialTable();
  size_t numG4MatCuts = theCoupleTable->GetTableSize();
  size_t numG4Mat     = theMaterialTable->size();
  //  theCoupleTable->DumpCouples();
  //  G4cout << *theMaterialTable;
  //
  // 0. count G4MaterialCutsCouple and unique G4Material objects that are used in 
  //    the courrent geometry. Record the indices of the used, unique materials.
  G4int  numUsedG4MatCuts = 0;
  G4int  numUsedG4Mat     = 0;
  std::vector<G4int> theUsedG4MatIndices(numG4Mat, -2);
  for (size_t imc=0; imc<numG4MatCuts; ++imc) {
    const G4MaterialCutsCouple* matCut = theCoupleTable->GetMaterialCutsCouple(imc);
    if (!matCut->IsUsed()) { 
      continue;
    }   
    ++numUsedG4MatCuts;
    size_t matIndx = matCut->GetMaterial()->GetIndex();
    if (theUsedG4MatIndices[matIndx]>-2) {
      continue;
    }
    // mark to be used in the geometry (but only once: numUsedG4Mat = unique used mats)
    theUsedG4MatIndices[matIndx] = -1;
    ++numUsedG4Mat;
  }  

  // 1. Allocate the MatCutData and MaterialData sub-structure of the global G4HepEmData 
  AllocateMatCutData   (&(hepEmData->fTheMatCutData), numG4MatCuts, numUsedG4MatCuts);
  AllocateMaterialData (&(hepEmData->fTheMaterialData), numG4Mat, numUsedG4Mat);
  AllocateElementData  (&(hepEmData->fTheElementData));
  // 2. Fill them in
  numUsedG4MatCuts = 0;
  numUsedG4Mat     = 0;
  for (size_t imc=0; imc<numG4MatCuts; ++imc) {
    const G4MaterialCutsCouple* matCut = theCoupleTable->GetMaterialCutsCouple(imc);
    if (!matCut->IsUsed()) { 
      continue;
    }    
    G4int mcIndx          = matCut->GetIndex();
    G4double gammaCutE    = (*(theCoupleTable->GetEnergyCutsVector(0)))[mcIndx];
    G4double electronCutE = (*(theCoupleTable->GetEnergyCutsVector(1)))[mcIndx];
    const G4Material* mat = matCut->GetMaterial();
    G4int matIndx         = mat->GetIndex();

    hepEmData->fTheMatCutData->fG4MCIndexToHepEmMCIndex[mcIndx] = numUsedG4MatCuts;
    struct G4HepEmMCCData& mccData = hepEmData->fTheMatCutData->fMatCutData[numUsedG4MatCuts];

    // NOTE: mccData.fHepEmMatIndex will be set below when it becomes known
    mccData.fG4MatCutIndex  = mcIndx;
    mccData.fSecElProdCutE  = std::max(electronCutE, hepEmPars->fElectronTranckingCut);
    mccData.fSecGamProdCutE = gammaCutE;
    mccData.fLogSecGamCutE  = std::log(gammaCutE);
    ++numUsedG4MatCuts;
    
    // check if the corresponding G4HepEm material struct has already been created
    if (theUsedG4MatIndices[matIndx]>-1) {
      // already created: 
      mccData.fHepEmMatIndex = theUsedG4MatIndices[matIndx];
    } else {
      // material structure has not been created so do it
      // take care of the corresponding element dat as well
      hepEmData->fTheMaterialData->fG4MatIndexToHepEmMatIndex[matIndx] = numUsedG4Mat;
      struct G4HepEmMatData& matData = hepEmData->fTheMaterialData->fMaterialData[numUsedG4Mat];
      //
      const G4ElementVector*  elmVec = mat->GetElementVector();
      const G4double* nAtomPerVolVec = mat->GetVecNbOfAtomsPerVolume();
      size_t            numOfElement = mat->GetNumberOfElements();
      //
      matData.fG4MatIndex              = matIndx;
      matData.fNumOfElement            = numOfElement;
      matData.fElementVect             = new int[numOfElement];
      matData.fNumOfAtomsPerVolumeVect = new double[numOfElement];
      matData.fDensity                 = mat->GetDensity();
      matData.fDensityCorFactor        = 4.0*CLHEP::pi*CLHEP::classic_electr_radius*CLHEP::electron_Compton_length*CLHEP::electron_Compton_length*mat->GetElectronDensity();
      matData.fElectronDensity         = mat->GetElectronDensity();
      //
      for (size_t ie=0; ie<numOfElement; ++ie) {
        G4int izet = ((*elmVec)[ie])->GetZasInt();
        matData.fElementVect[ie] = izet;
        matData.fNumOfAtomsPerVolumeVect[ie] = nAtomPerVolVec[ie];      
        // fill element data as well if haven't done yet
        izet = std::min ( izet, (G4int)hepEmData->fTheElementData->fMaxZet );
        struct G4HepEmElemData& elData = hepEmData->fTheElementData->fElementData[izet];
        if (elData.fZet<0) {
          elData.fZet   = (double)izet;
          elData.fZet13 = std::pow(elData.fZet, 1.0/3.0); 
        }        
      }
      //
      theUsedG4MatIndices[matIndx] = numUsedG4Mat;
      mccData.fHepEmMatIndex = theUsedG4MatIndices[matIndx];
      ++numUsedG4Mat;
    }
  }
}
