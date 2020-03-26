#pragma once

#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <zenload/zTypes.h>
#include <vector>
#include <cstdint>

#include "graphics/protomesh.h"

class PhysicMesh:public btTriangleIndexVertexArray {
  public:
    PhysicMesh(ZenLoad::PackedMesh&& sPacked);
    PhysicMesh(const std::vector<btVector3>* v);

    PhysicMesh(const PhysicMesh&)=delete;
    PhysicMesh(PhysicMesh&&)=delete;

    void    addIndex(std::vector<uint32_t>&& index, uint8_t material);
    void    addIndex(std::vector<uint32_t>&& index, uint8_t material, const char* sector);
    uint8_t getMaterialId(size_t segment) const;
    auto    getSectorName(size_t segment) const -> const char*;
    bool    useQuantization() const;
    bool    isEmpty() const;

    void    adjustMesh();

  private:
    PhysicMesh(const std::vector<ZenLoad::WorldVertex>& v);

    void addSegment(size_t indexSize,size_t offset,uint8_t material,const char* sector);

    struct Segment {
      size_t      off;
      int         size;
      uint8_t     mat;
      const char* sector=nullptr;
      };

    std::vector<btVector3>        vStorage;
    const std::vector<btVector3>& vert;
    std::vector<uint32_t>         id;
    std::vector<Segment>          segments;
  };
