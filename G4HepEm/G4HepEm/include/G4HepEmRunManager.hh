

#ifndef G4HepEmRunManager_HH
#define G4HepEmRunManager_HH



// forward declare
struct G4HepEmData;
struct G4HepEmParameters;

class  G4HepEmTLData;
class  G4HepEmElectronManager;
class  G4HepEmGammaManager;

class  G4HepEmRandomEngine;

#include <vector>


/**
 * @file    G4HepEmRunManager.hh
 * @class   G4HepEmRunManager
 * @author  M. Novak
 * @date    2020
 *
 * This is the top level interface to all G4HepEm functionalities.
 *
 * A master object is responsible to construct, store and initialise all global
 * (e.g. material, material-cut, element, configuartion parameter, etc) related
 * data structures used by (at run-time), and shared among all other (worker)
 * run managers. These need to be done only once for a given run: only once for
 * all (e-/e+ and gamma) particles and only by the master.
 * Additional data structures are aslo created by the master run manager that
 * are also shared between the workers. These are data, specific for a given
 * particle type and needs to be crated only if it's required i.e. if the
 * simulation of that particle needs to be done by the HepEm. These need to be
 * created and initialised only by the master and individually for each particle
 * when requested.
 * All the above data objects are constructed by and stored in the master run
 * manager and worker run managers will have only their pointer mebers to set to
 * these unique data objects (used as read-only at run time).
 * Beyond these shared data objects, each worker run-manager will have their own
 * instance from the G4HepEmTLData that stores worker local data.
 */



class G4HepEmRunManager {

public:

  G4HepEmRunManager (bool ismaster);
 ~G4HepEmRunManager ();

  static G4HepEmRunManager* GetMasterRunManager ();

  /**
   * Builds or sets (data) members of run-manager.
   *
   * For the master-RM it initialises i.e. builds:
   * - the `global` data structures that are shared by all run-managers i.e.
   *   shared by all workers, processes and particles at run-time as read-only
   *   data (configuration parameters, all the elememnt, material and material-
   *   production-cuts related data structres).
   * - particle specific data that are also shared by all run-managers i.e. by
   *   all workers (range, dE/dx tables for e-/e+, macroscopic cross sections
   *   and target element selectors)
   * For a worker-RM:
   * - sets all the pointer members to data that are shared among the run-managers
   *   to their master values.
   * - creates and sets the worker-local data structure for each worker and sets
   *   its random engine pointer to the corresponding geant4, thread local random
   *   engine pointer.
   */
  void Initialize (G4HepEmRandomEngine* theRNGEngine, int hepEmParticleIndx, G4HepEmParameters* hepEmPars=nullptr);


  /**
   * Clears all data structures that has been created by calling the Initialize()
   * method and re-sets the correspondig pointer members to null.
   */
  void Clear ();

  /** delete copy CTR and assigment operators */
  G4HepEmRunManager (const G4HepEmRunManager&) = delete;
  G4HepEmRunManager& operator= (const G4HepEmRunManager&) = delete;

 //  void SetHepEmData(struct G4HepEmData* hepEmData) { fTheG4HepEmData; }

  struct G4HepEmData*       GetHepEmData()         const  { return fTheG4HepEmData; }
  struct G4HepEmParameters* GetHepEmParameters()   const  { return fTheG4HepEmParameters; }
  G4HepEmTLData*            GetTheTLData()         const  { return fTheG4HepEmTLData; }

  void SetVerbose(int verbose) { fVerbose = verbose; }

private:

  /**
   * Initialisation of all global data structures.
   *
   * Extracts EM and other configuration parameters, builds all the elememnt,
   * matrial and material-production-cuts related data structures shared by all
   * workers, all processes and all particles at run-time as read-only data. In
   * other words, these are the `global` data structures.
   * This should be invoked by the master thread and only once!
   */
  void InitializeGlobal (G4HepEmParameters* hepEmPars=nullptr);




private:
  /** Flag to indicate the master run-manager.*/
  bool                           fIsMaster;
  /** Flags to indicate if the master has been initialized for the given particle.*/
  bool                           fIsInitialisedForParticle[3];
  /** Pointer to the master run-manager.*/
  static G4HepEmRunManager*      gTheG4HepEmRunManagerMaster;
  /**
   * === These data are created by the Master-RM and shared among all Worker-RMs.
   *
   * Collection of configuration parameters used at initialization and run time.
   */
  struct G4HepEmParameters*      fTheG4HepEmParameters;
  bool   fExternalParameters;
  /*
   * The top level data structure that stores all the data used by all processes
   * (e.g. material or material cuts couple related data, etc.)
   * The coresponding data structures are created and filled when calling the
   * Initialize() method
   */
  struct G4HepEmData*            fTheG4HepEmData;

  /*
   * Processes for e-/e+: this is the top level object to all e-/e+ related
   * interactions that can provide response to how far the particle goes till the
   * next interaction and what happens in that interaction.
   *
   *
   * === These data are created for each Worker-RM.
   */

  G4HepEmTLData*                 fTheG4HepEmTLData;

  // Verbosity level (only 0/1 at the moment)
  int fVerbose;

};

#endif // G4HepEmRunManager_HH
