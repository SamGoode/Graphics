#pragma once

#include <cassert>
#include <bitset>


namespace ECS {
	#define MAX_ENTITIES 100
	#define MAX_COMPONENT_TYPES 10

	using uint = unsigned int; // can replace later with lower memory type
	using bitset = std::bitset<MAX_COMPONENT_TYPES>; // maybe same name is bad idea?

	class EntityManager {
	private:
		//const uint maxEntities = MAX_ENTITIES;

		uint activeCount = 0;
		uint inactiveEntities[MAX_ENTITIES];
		bitset entitySignatures[MAX_ENTITIES];
		//uint entityIDs[MAX_ENTITIES];
		//uint entityToIndex[MAX_ENTITIES];

	public:
		EntityManager() {
			for (uint i = 0; i < MAX_ENTITIES; i++) {
				//entityIDs[i] = i;
				inactiveEntities[i] = i;
			}
		}

		uint createEntity() {
			assert(activeCount < MAX_ENTITIES);

			//uint newEntity = entityIDs[activeEntities];
			//entityToIndex[newEntity] = activeEntities;
			//uint newEntity = inactiveEntities[maxEntities - activeCount];

			activeCount++;
			return inactiveEntities[MAX_ENTITIES - activeCount];
		}

		void destroyEntity(uint entityID) {
			assert(entityID < MAX_ENTITIES && activeCount > 0);
			//uint entityID = entityIDs[index];
			entitySignatures[entityID].reset();

			inactiveEntities[MAX_ENTITIES - activeCount] = entityID;
			activeCount--;
		}

		void setSignature(uint entityID, bitset signature) {
			entitySignatures[entityID] = signature;
		}

		bitset getSignature(uint entityID) {
			return entitySignatures[entityID];
		}
	};



	class IComponentPool {
	public:
		virtual ~IComponentPool() = default;

		virtual void onEntityDestroyed(uint entityID) = 0;
	};

	template<typename T>
	class ComponentPool : public IComponentPool {
	private:
		uint activeComponents = 0;
		T components[MAX_ENTITIES];

		// Apparently this forms a sparse integer set
		uint entityToIndex[MAX_ENTITIES];
		uint indexToEntity[MAX_ENTITIES];

	public:
		ComponentPool() {
			// May be unnecessary
			for (uint i = 0; i < MAX_ENTITIES; i++) {
				entityToIndex[i] = i;
				indexToEntity[i] = i;
			}
		}

		void insertData(uint entityID, T component) {
			assert(entityID < MAX_ENTITIES && entityToIndex[entityID] >= activeComponents && "Entity already has this component");
			components[activeComponents] = component;

			// this part may be unnecessary
			uint oldIndex = entityToIndex[entityID];
			uint oldEntity = indexToEntity[activeComponents];
			indexToEntity[oldIndex] = oldEntity;
			entityToIndex[oldEntity] = oldIndex;
			//---------------------------------


			indexToEntity[activeComponents] = entityID;
			entityToIndex[entityID] = activeComponents;
			activeComponents++;
		}

		void removeData(uint entityID) {
			assert(entityID < MAX_ENTITIES && entityToIndex[entityID] < activeComponents);

			activeComponents--;

			//uint lastIndex = activeComponents;
			uint removedIndex = entityToIndex[entityID];
			uint lastEntity = indexToEntity[activeComponents];

			components[removedIndex] = components[activeComponents];

			indexToEntity[removedIndex] = lastEntity;
			entityToIndex[lastEntity] = removedIndex;

			// this part may be unnecessary
			indexToEntity[activeComponents] = entityID;
			entityToIndex[entityID] = activeComponents;
		}

		T& getData(uint entityID) {
			assert(entityID < MAX_ENTITIES && entityToIndex[entityID] < activeComponents);

			return components[entityToIndex[entityID]];
		}

		virtual void onEntityDestroyed(uint entityID) override {
			if (indexToEntity[entityToIndex[entityID]] == entityID && entityToIndex[entityID] < activeComponents) {
				removeData(entityID);
			}
		}
	};


	template<typename T>
	struct IComponentType {
		static inline bool isRegistered;
		static inline uint componentID;
	};


	class ComponentManager {
	private:
		uint componentTypes = 0;
		IComponentPool* componentPools[MAX_COMPONENT_TYPES];

	public:
		~ComponentManager() {
			for (uint i = 0; i < componentTypes; i++) {
				delete componentPools[i];
			}
		}

		template<typename T>
		void registerComponent() {
			assert(!IComponentType<T>::isRegistered && "Component already registered");

			componentPools[componentTypes] = new ComponentPool<T>();
			IComponentType<T>::componentID = componentTypes;
			IComponentType<T>::isRegistered = true;

			componentTypes++;
		}

		template<typename T>
		void addComponent(uint entityID, T component) {
			getComponentPool<T>()->insertData(entityID, component);
		}

		template<typename T>
		void removeComponent(uint entityID) {
			getComponentPool<T>()->removeData(entityID);
		}

		template<typename T>
		T& getComponent(uint entityID) {
			return getComponentPool<T>()->getData(entityID);
		}

		template<typename T>
		uint getComponentID() {
			assert(IComponentType<T>::isRegistered && "Component is not registered");

			return IComponentType<T>::componentID;
		}

		void onEntityDestroyed(uint entityID) {
			for (uint i = 0; i < componentTypes; i++) {
				componentPools[i]->onEntityDestroyed(entityID);
			}
		}

	private:
		template<typename T>
		ComponentPool<T>* getComponentPool() {
			uint componentID = getComponentID<T>();
			return static_cast<ComponentPool<T>*>(componentPools[componentID]);
		}
	};



	class System {
	public:
		uint entityCount = 0;
		uint entities[MAX_ENTITIES];
		bitset signature;

	public:
		virtual ~System() = default;

		void addEntity(uint entityID) {
			entities[entityCount++] = entityID;
		}

		void removeEntity(uint entityID) {
			for (uint i = 0; i < entityCount; i++) {
				if (entities[i] == entityID) {
					entities[i] = entities[--entityCount];
				}
			}
		}
	};

	template<typename T>
	struct ISystemType {
		static inline bool isRegistered;
		static inline uint systemID;
	};

	class SystemManager {
	private:
		uint systemsCount = 0;
		System* systems[10];

	public:
		template<typename T>
		System* registerSystem() {
			assert(!ISystemType<T>::isRegistered && "System already registered");

			uint systemID = systemsCount++;
			systems[systemID] = new T();
			ISystemType<T>::isRegistered = true;
			ISystemType<T>::systemID = systemID;

			return systems[systemID];
		}

		template<typename T>
		void setSignature(bitset _signature) {
			getSystem<T>()->signature = _signature;
		}

		template<typename T>
		T* getSystem() {
			return static_cast<T*>(systems[ISystemType<T>::systemID]);
		}

		void onEntityDestroyed(uint entityID) {
			for (int i = 0; i < systemsCount; i++) {
				systems[i]->removeEntity(entityID);
			}
		}

		void onEntitySignatureChanged(uint entityID, bitset entitySignature) {
			for (int i = 0; i < systemsCount; i++) {
				bitset signature = systems[i]->signature;
				if ((signature & entitySignature) == signature) {
					systems[i]->addEntity(entityID);
				}
				else {
					systems[i]->removeEntity(entityID);
				}
			}
		}
	};


	class ECSManager {
	private:
		EntityManager* entityManager = nullptr;
		ComponentManager* componentManager = nullptr;
		SystemManager* systemManager = nullptr;

	public:
		~ECSManager() {
			delete entityManager;
			delete componentManager;
			delete systemManager;
		}

		void init() {
			assert(entityManager == nullptr && componentManager == nullptr && systemManager == nullptr);

			entityManager = new EntityManager();
			componentManager = new ComponentManager();
			systemManager = new SystemManager();
		}

		// Entity Methods
		uint createEntity() {
			return entityManager->createEntity();
		}

		void destroyEntity(uint entityID) {
			entityManager->destroyEntity(entityID);
			componentManager->onEntityDestroyed(entityID);
			systemManager->onEntityDestroyed(entityID);
		}

		// Component Methods
		template<typename T>
		void registerComponent() {
			componentManager->registerComponent<T>();
		}

		template<typename T>
		void addComponent(uint entityID, T component) {
			componentManager->addComponent<T>(entityID, component);

			bitset signature = entityManager->getSignature(entityID);
			signature.set(componentManager->getComponentID<T>(), true);
			entityManager->setSignature(entityID, signature);

			systemManager->onEntitySignatureChanged(entityID, signature);
		}

		template<typename T>
		void removeComponent(uint entityID) {
			componentManager->removeComponent<T>(entityID);

			bitset signature = entityManager->getSignature(entityID);
			signature.set(componentManager->getComponentID<T>(), false);
			entityManager->setSignature(entityID, signature);

			systemManager->onEntitySignatureChanged(entityID, signature);
		}

		template<typename T>
		T& getComponent(uint entityID) {
			return componentManager->getComponent<T>(entityID);
		}

		template<typename T>
		uint getComponentID() {
			return componentManager->getComponentID<T>();
		}

		// System Methods
		template<typename T>
		System* registerSystem() {
			return systemManager->registerSystem<T>();
		}

		template<typename T>
		void setSystemSignature(bitset signature) {
			systemManager->setSignature<T>(signature);
		}

		template<typename T>
		T* getSystem() {
			return systemManager->getSystem<T>();
		}
	};

}