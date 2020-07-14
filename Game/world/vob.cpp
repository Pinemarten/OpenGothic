#include "vob.h"

#include <Tempest/Log>
#include <Tempest/Vec>

#include "interactive.h"
#include "staticobj.h"

#include "world/triggers/movetrigger.h"
#include "world/triggers/codemaster.h"
#include "world/triggers/triggerlist.h"
#include "world/triggers/triggerscript.h"
#include "world/triggers/triggerworldstart.h"
#include "world/triggers/zonetrigger.h"
#include "world/triggers/messagefilter.h"
#include "world/triggers/trigger.h"

#include "world.h"

using namespace Tempest;

Vob::Vob(World& owner)
  : world(owner) {
  }

Vob::Vob(Vob* parent, World& owner, ZenLoad::zCVobData& vob, bool startup)
  : world(owner), parent(parent) {
  float v[16]={};
  std::memcpy(v,vob.worldMatrix.m,sizeof(v));
  pos   = Tempest::Matrix4x4(v);
  local = pos;

  if(parent!=nullptr) {
    auto m = parent->transform();
    m.inverse();
    m.mul(local);
    local = m;
    }

  for(auto& i:vob.childVobs) {
    auto p = Vob::load(this,owner,std::move(i),startup);
    if(p!=nullptr)
      child.emplace_back(std::move(p));
    }
  vob.childVobs.clear();
  }

Vob::~Vob() {
  }

Vec3 Vob::position() const {
  return Vec3(pos.at(3,0),pos.at(3,1),pos.at(3,2));
  }

void Vob::setGlobalTransform(const Matrix4x4& p) {
  pos   = p;
  local = pos;

  if(parent!=nullptr) {
    auto m = parent->transform();
    m.inverse();
    m.mul(local);
    local = m;
    }
  }

void Vob::setLocalTransform(const Matrix4x4& p) {
  local = p;
  recalculateTransform();
  }

void Vob::moveEvent() {
  }

void Vob::recalculateTransform() {
  auto old = position();
  if(parent!=nullptr) {
    pos = parent->transform();
    pos.mul(local);
    } else {
    pos = local;
    }
  if(old != position()) {
    world.invalidateVobIndex();
    }
  moveEvent();
  for(auto& i:child) {
    i->recalculateTransform();
    }
  }


std::unique_ptr<Vob> Vob::load(Vob* parent, World& world, ZenLoad::zCVobData&& vob, bool startup) {
  switch(vob.vobType) {
    case ZenLoad::zCVobData::VT_zCVob:
      return std::unique_ptr<Vob>(new StaticObj(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCVobLevelCompo:
      return std::unique_ptr<Vob>(new Vob(parent,world,vob,startup));
    case ZenLoad::zCVobData::VT_oCMobFire:
      return std::unique_ptr<Vob>(new StaticObj(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCMOB:
      // Irdotar bow-triggers
      // focusOverride=true
      return std::unique_ptr<Vob>(new Interactive(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCMobBed:
    case ZenLoad::zCVobData::VT_oCMobDoor:
    case ZenLoad::zCVobData::VT_oCMobInter:
    case ZenLoad::zCVobData::VT_oCMobContainer:
    case ZenLoad::zCVobData::VT_oCMobSwitch:
      return std::unique_ptr<Vob>(new Interactive(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCMobLadder:
      //TODO: mob ladder
      return std::unique_ptr<Vob>(new StaticObj(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCMobWheel:
      //TODO: mob whell
      return std::unique_ptr<Vob>(new StaticObj(parent,world,std::move(vob),startup));

    case ZenLoad::zCVobData::VT_zCMover:
      return std::unique_ptr<Vob>(new MoveTrigger(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCCodeMaster:
      return std::unique_ptr<Vob>(new CodeMaster(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCTriggerList:
      return std::unique_ptr<Vob>(new TriggerList(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCTriggerScript:
      return std::unique_ptr<Vob>(new TriggerScript(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCTriggerWorldStart:
      return std::unique_ptr<Vob>(new TriggerWorldStart(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_oCTriggerChangeLevel:
      return std::unique_ptr<Vob>(new ZoneTrigger(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCTrigger:
      return std::unique_ptr<Vob>(new Trigger(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCMessageFilter:
      return std::unique_ptr<Vob>(new MessageFilter(parent,world,std::move(vob),startup));
    case ZenLoad::zCVobData::VT_zCVobStartpoint: {
      float dx = vob.rotationMatrix.v[2].x;
      float dy = vob.rotationMatrix.v[2].y;
      float dz = vob.rotationMatrix.v[2].z;
      world.addStartPoint(Vec3(vob.position.x,vob.position.y,vob.position.z),Vec3(dx,dy,dz),vob.vobName.c_str());
      return nullptr;
      }
    case ZenLoad::zCVobData::VT_zCVobSpot: {
      float dx = vob.rotationMatrix.v[2].x;
      float dy = vob.rotationMatrix.v[2].y;
      float dz = vob.rotationMatrix.v[2].z;
      world.addFreePoint(Vec3(vob.position.x,vob.position.y,vob.position.z),Vec3(dx,dy,dz),vob.vobName.c_str());
      return nullptr;
      }
    case ZenLoad::zCVobData::VT_oCItem: {
      if(startup)
        world.addItem(vob);
      return nullptr;
      }
    case ZenLoad::zCVobData::VT_zCVobSound:
    case ZenLoad::zCVobData::VT_zCVobSoundDaytime:
    case ZenLoad::zCVobData::VT_oCZoneMusic:
    case ZenLoad::zCVobData::VT_oCZoneMusicDefault: {
      world.addSound(vob);
      return nullptr;
      }
    case ZenLoad::zCVobData::VT_zCVobLight: {
      world.addLight(vob);
      return nullptr;
      }
    }

  if(vob.objectClass=="zCVobAnimate:zCVob" || // ork flags
     vob.objectClass=="zCPFXControler:zCVob") {
    //TODO: morph animation
    return std::unique_ptr<Vob>(new StaticObj(parent,world,std::move(vob),startup));
    }
  else if(vob.objectClass=="oCTouchDamage:zCTouchDamage:zCVob") {
    // NOT IMPLEMENTED
    }
  else if(vob.objectClass=="zCVobLensFlare:zCVob" ||
          vob.objectClass=="zCZoneVobFarPlane:zCVob" ||
          vob.objectClass=="zCZoneVobFarPlaneDefault:zCZoneVobFarPlane:zCVob" ||
          vob.objectClass=="zCZoneZFog:zCVob" ||
          vob.objectClass=="zCZoneZFogDefault:zCZoneZFog:zCVob") {
    // WONT-IMPLEMENT
    }
  else {
    static std::unordered_set<std::string> cls;
    if(cls.find(vob.objectClass)==cls.end()){
      cls.insert(vob.objectClass);
      Tempest::Log::d("unknown vob class ",vob.objectClass);
      }
    }
  return nullptr;
  }
