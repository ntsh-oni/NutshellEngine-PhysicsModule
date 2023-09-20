#pragma once
#include "../Common/module_interfaces/ntshengn_physics_module_interface.h"
#include "../Common/ecs/ntshengn_ecs.h"
#include "../Common/utils/ntshengn_utils_math.h"
#include <set>
#include <vector>
#include <unordered_map>

struct RigidbodyState {
	NtshEngn::Math::vec3 acceleration = NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f);
	NtshEngn::Math::vec3 velocity = NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f);
};

struct EntityAABB {
	NtshEngn::Math::vec3 position;
	NtshEngn::Math::vec3 size;
};

struct BroadphaseCollision {
	NtshEngn::Entity entity1;
	NtshEngn::Entity entity2;

	bool operator<(const BroadphaseCollision& other) const {
		size_t hash = static_cast<size_t>(entity1) + (static_cast<size_t>(entity2) << 8);
		size_t otherHash = static_cast<size_t>(other.entity1) + (static_cast<size_t>(other.entity2) << 8);

		return hash < otherHash;
	}
};

struct NarrowphaseCollision {
	NtshEngn::Entity entity1;
	NtshEngn::Entity entity2;
	NtshEngn::Math::vec3 intersectionNormal;
	float intersectionDepth;
};

class GJKSimplex {
public:
	GJKSimplex& operator=(std::initializer_list<NtshEngn::Math::vec3> list) {
		for (std::initializer_list<NtshEngn::Math::vec3>::iterator it = list.begin(); it != list.end(); it++) {
			m_points[std::distance(list.begin(), it)] = *it;
		}
		m_size = list.size();

		return *this;
	}

	void push_front(const NtshEngn::Math::vec3& point) {
		m_points = { point, m_points[0], m_points[1], m_points[2] };
		m_size = std::min(m_size + 1, static_cast<size_t>(4));
	}

	NtshEngn::Math::vec3& operator[](size_t i) {
		return m_points[i];
	}

	size_t size() {
		return m_size;
	}

	std::array<NtshEngn::Math::vec3, 4>::iterator begin() {
		return m_points.begin();
	}

	std::array<NtshEngn::Math::vec3, 4>::iterator end() {
		return m_points.end() - (4 - m_size);
	}

private:
	std::array<NtshEngn::Math::vec3, 4> m_points = { NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f),
		NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f),
		NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f),
		NtshEngn::Math::vec3(0.0f, 0.0f, 0.0f) };
	size_t m_size = 0;
};

namespace NtshEngn {

	class PhysicsModule : public PhysicsModuleInterface {
	public:
		PhysicsModule() : PhysicsModuleInterface("NutshellEngine Euler Physics Module") {}

		void init();
		void update(double dt);
		void destroy();

		// Returns an IntersectionInformation structure containing information about the intersection
		IntersectionInformation intersect(const ColliderShape* shape1, const ColliderShape* shape2);

		// Returns a list of RaycastInformation structures containing information about the hit entities
		std::vector<RaycastInformation> raycast(const Math::vec3& rayOrigin, const Math::vec3& rayDirection, float tMin, float tMax);

	public:
		const ComponentMask getComponentMask() const;

		void onEntityComponentAdded(Entity entity, Component componentID);
		void onEntityComponentRemoved(Entity entity, Component componentID);

	private:
		void eulerIntegrator(float dtSeconds);
		void collisionsDetection();
		void collisionsResponse();

		void collisionsBroadphase();
		void collisionsNarrowphase();

		IntersectionInformation intersect(const ColliderSphere* sphere1, const ColliderSphere* sphere2);
		IntersectionInformation intersect(const ColliderSphere* sphere, const ColliderAABB* aabb);
		IntersectionInformation intersect(const ColliderSphere* sphere, const ColliderOBB* obb);
		IntersectionInformation intersect(const ColliderSphere* sphere, const ColliderCapsule* capsule);
		IntersectionInformation intersect(const ColliderAABB* aabb1, const ColliderAABB* aabb2);
		IntersectionInformation intersect(const ColliderAABB* aabb, const ColliderOBB* obb);
		IntersectionInformation intersect(const ColliderAABB* aabb, const ColliderCapsule* capsule);
		IntersectionInformation intersect(const ColliderOBB* obb1, const ColliderOBB* obb2);
		IntersectionInformation intersect(const ColliderOBB* obb, const ColliderCapsule* capsule);
		IntersectionInformation intersect(const ColliderCapsule* capsule1, const ColliderCapsule* capsule2);

		IntersectionInformation intersect(const ColliderCapsule* capsule, const ColliderSphere* sphere);
		IntersectionInformation intersect(const ColliderOBB* obb, const ColliderSphere* sphere);
		IntersectionInformation intersect(const ColliderAABB* aabb, const ColliderSphere* sphere);
		IntersectionInformation intersect(const ColliderCapsule* capsule, const ColliderOBB* obb);
		IntersectionInformation intersect(const ColliderCapsule* capsule, const ColliderAABB* aabb);
		IntersectionInformation intersect(const ColliderOBB* obb, const ColliderAABB* aabb);

		IntersectionInformation gjk(const ColliderShape* shape1, const ColliderShape* shape2);

		bool simplexContainsOrigin(GJKSimplex& simplex, NtshEngn::Math::vec3& direction);

		bool gjkLine(GJKSimplex& simplex, Math::vec3& direction);
		bool gjkTriangle(GJKSimplex& simplex, Math::vec3& direction);
		bool gjkTetrahedron(GJKSimplex& simplex, Math::vec3& direction);

		bool sameDirection(const Math::vec3& a, const Math::vec3& b);

		Math::vec3 getCenter(const ColliderShape* shape);
		Math::vec3 getCenter(const ColliderSphere* sphere);
		Math::vec3 getCenter(const ColliderAABB* aabb);
		Math::vec3 getCenter(const ColliderOBB* obb);
		Math::vec3 getCenter(const ColliderCapsule* capsule);

		Math::vec3 support(const ColliderShape* shape1, const ColliderShape* shape2, const Math::vec3& direction);

		Math::vec3 getFarthestPointInDirection(const ColliderShape* shape, const Math::vec3& direction);
		Math::vec3 getFarthestPointInDirection(const ColliderSphere* sphere, const Math::vec3& direction);
		Math::vec3 getFarthestPointInDirection(const ColliderAABB* aabb, const Math::vec3& direction);
		Math::vec3 getFarthestPointInDirection(const ColliderOBB* obb, const Math::vec3& direction);
		Math::vec3 getFarthestPointInDirection(const ColliderCapsule* capsule, const Math::vec3& direction);

		std::pair<Math::vec3, float> epa(const ColliderShape* shape1, const ColliderShape* shape2, GJKSimplex& simplex);

		std::pair<std::vector<Math::vec4>, size_t> getPolytopeNormals(const std::vector<Math::vec3>& polytope, const std::vector<size_t>& faces);
		void addIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& polytopeIndices, size_t a, size_t b);

		void transform(ColliderShape* shape, const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale);
		void transform(ColliderSphere* sphere, const Math::vec3& translation, const Math::vec3& scale);
		void transform(ColliderAABB* aabb, const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale);
		void transform(ColliderOBB* obb, const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale);
		void transform(ColliderCapsule* capsule, const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale);

		Math::vec3 closestPointOnSegment(const Math::vec3& point, const Math::vec3& segmentA, const Math::vec3& segmentB);
		std::pair<Math::vec3, Math::vec3> closestPointSegmentSegment(const Math::vec3& segmentA1, const Math::vec3& segmentA2, const Math::vec3& segmentB1, const Math::vec3& segmentB2);

	private:
		const uint32_t maxIterations = 60;
		const double maxDeltaTime = 1000.0 / 60.0;
		double timeAccumulator = 0.0;

		const Math::vec3 m_gravity = Math::vec3(0.0f, -9.81f, 0.0f);

		std::set<BroadphaseCollision> m_broadphaseCollisions;
		std::vector<NarrowphaseCollision> m_narrowphaseCollisions;

		std::unordered_map<Entity, RigidbodyState> m_rigidbodyStates;
	};

}