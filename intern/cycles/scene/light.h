/* SPDX-FileCopyrightText: 2011-2022 Blender Foundation
 *
 * SPDX-License-Identifier: Apache-2.0 */

#pragma once

#include "kernel/types.h"

#include "graph/node.h"

/* included as Light::set_shader defined through NODE_SOCKET_API does not select
 * the right Node::set overload as it does not know that Shader is a Node */
#include "scene/shader.h"

#include "util/ies.h"
#include "util/thread.h"
#include "util/types.h"
#include "util/unique_ptr.h"
#include "util/vector.h"

CCL_NAMESPACE_BEGIN

class Device;
class DeviceScene;
class Progress;
class Scene;
class Shader;

class Light : public Node {
 public:
  NODE_DECLARE;

  Light();

  NODE_SOCKET_API(LightType, light_type)
  NODE_SOCKET_API(float3, strength)

  NODE_SOCKET_API(float, size)
  NODE_SOCKET_API(float, angle)

  NODE_SOCKET_API(float, sizeu)
  NODE_SOCKET_API(float, sizev)
  NODE_SOCKET_API(bool, ellipse)
  NODE_SOCKET_API(float, spread)

  NODE_SOCKET_API(Transform, tfm)

  NODE_SOCKET_API(int, map_resolution)
  NODE_SOCKET_API(float, average_radiance)

  NODE_SOCKET_API(bool, is_sphere)

  NODE_SOCKET_API(float, spot_angle)
  NODE_SOCKET_API(float, spot_smooth)

  NODE_SOCKET_API(bool, cast_shadow)
  NODE_SOCKET_API(bool, use_mis)
  NODE_SOCKET_API(bool, use_camera)
  NODE_SOCKET_API(bool, use_diffuse)
  NODE_SOCKET_API(bool, use_glossy)
  NODE_SOCKET_API(bool, use_transmission)
  NODE_SOCKET_API(bool, use_scatter)
  NODE_SOCKET_API(bool, use_caustics)

  NODE_SOCKET_API(bool, is_shadow_catcher)
  NODE_SOCKET_API(bool, is_portal)
  NODE_SOCKET_API(bool, is_enabled)

  NODE_SOCKET_API(Shader *, shader)
  NODE_SOCKET_API(int, max_bounces)
  NODE_SOCKET_API(uint, random_id)

  NODE_SOCKET_API(ustring, lightgroup)
  NODE_SOCKET_API(uint64_t, light_set_membership);
  NODE_SOCKET_API(uint64_t, shadow_set_membership);

  /* Normalize power by the surface area of the light. */
  NODE_SOCKET_API(bool, normalize)

  void tag_update(Scene *scene);

  /* Check whether the light has contribution the scene. */
  bool has_contribution(Scene *scene);

  /* Check whether this light participates in light or shadow linking. */
  bool has_light_linking() const;
  bool has_shadow_linking() const;

  /* Convenience access to transform. */
  float3 get_co() const;
  float3 get_dir() const;
  float3 get_axisu() const;
  float3 get_axisv() const;

  friend class LightManager;
  friend class LightTree;
};

class LightManager {
 public:
  enum : uint32_t {
    MESH_NEED_REBUILD = (1 << 0),
    EMISSIVE_MESH_MODIFIED = (1 << 1),
    LIGHT_MODIFIED = (1 << 2),
    LIGHT_ADDED = (1 << 3),
    LIGHT_REMOVED = (1 << 4),
    OBJECT_MANAGER = (1 << 5),
    SHADER_COMPILED = (1 << 6),
    SHADER_MODIFIED = (1 << 7),

    /* tag everything in the manager for an update */
    UPDATE_ALL = ~0u,

    UPDATE_NONE = 0u,
  };

  /* Need to update background (including multiple importance map) */
  bool need_update_background;

  LightManager();

  /* IES texture management */
  int add_ies(const string &content);
  int add_ies_from_file(const string &filename);
  void remove_ies(const int slot);

  void device_update(Device *device, DeviceScene *dscene, Scene *scene, Progress &progress);
  void device_free(Device *device, DeviceScene *dscene, const bool free_background = true);

  void tag_update(Scene *scene, const uint32_t flag);

  bool need_update() const;

  /* Check whether there is a background light. */
  bool has_background_light(Scene *scene);

 protected:
  /* Optimization: disable light which is either unsupported or
   * which doesn't contribute to the scene or which is only used for MIS
   * and scene doesn't need MIS.
   */
  void test_enabled_lights(Scene *scene);

  void device_update_lights(DeviceScene *dscene, Scene *scene);
  void device_update_distribution(Device *device,
                                  DeviceScene *dscene,
                                  Scene *scene,
                                  Progress &progress);
  void device_update_tree(Device *device, DeviceScene *dscene, Scene *scene, Progress &progress);
  void device_update_background(Device *device,
                                DeviceScene *dscene,
                                Scene *scene,
                                Progress &progress);
  void device_update_ies(DeviceScene *dscene);

  struct IESSlot {
    IESFile ies;
    uint hash;
    int users;
  };

  vector<unique_ptr<IESSlot>> ies_slots;
  thread_mutex ies_mutex;

  bool last_background_enabled;
  int last_background_resolution;

  uint32_t update_flags;
};

CCL_NAMESPACE_END
