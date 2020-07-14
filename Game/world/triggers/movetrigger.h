#pragma once

#include "graphics/meshobjects.h"
#include "physics/physicmesh.h"
#include "abstracttrigger.h"

class MoveTrigger : public AbstractTrigger {
  public:
    MoveTrigger(Vob* parent, World &world, ZenLoad::zCVobData&& data, bool startup);

    void onTrigger(const TriggerEvent& evt) override;
    void onUntrigger(const TriggerEvent& evt) override;
    bool hasVolume() const override;
    void tick(uint64_t dt) override;

  private:
    void moveEvent() override;
    void processTrigger(const TriggerEvent& evt, bool onTrigger);

    void setView     (MeshObjects::Mesh&& m);
    void emitSound   (const char* snd, bool freeSlot=true);
    void advanceAnim (uint32_t f0, uint32_t f1, float alpha);

    enum State : int32_t {
      Idle         = 0,
      Loop         = 1,
      Open         = 2,
      OpenTimed    = 3,
      Close        = 4,
      NextKey      = 5,
      };

    Tempest::Matrix4x4       pos0;
    MeshObjects::Mesh        view;
    PhysicMesh               physic;

    State                    state     = Idle;
    uint64_t                 sAnim     = 0;

    uint32_t                 frame     = 0;
  };
