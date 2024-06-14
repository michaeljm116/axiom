#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <stdexcept>
#include <span>

namespace Axiom{

    namespace Memory{

        template<typename Resource>
        class Manager {
        private:
            std::vector<Resource> resources;
            std::unordered_map<std::string, size_t> resourceMap;
            std::queue<size_t> freeIndices;  // Queue to track free slots in the resources vector
            size_t last_added;
        public:
            // Add a new resource or replace an existing one
            size_t add_resource(const Resource& resource, const std::string& name) {
                auto it = resourceMap.find(name);
                if (it != resourceMap.end()) {
                    // Replace existing resource
                    resources[it->second] = resource;
                    last_added = it->second;
                    return it->second;
                }

                size_t index;
                if (!freeIndices.empty()) {
                    index = freeIndices.front();  // Reuse a free index
                    freeIndices.pop();
                    resources[index] = resource;  // Place new resource at the freed slot
                } else {
                    resources.push_back(resource); // No free slot, add new resource
                    index = resources.size() - 1;
                }
                resourceMap[name] = index;
                last_added = index;
                return index;
            }

            // Retrieve a resource by name
            Resource& get_resource(const std::string& name) {
                auto it = resourceMap.find(name);
                if (it == resourceMap.end()) {
                    throw std::runtime_error("Resource not found.");
                }
                return resources[it->second];
            }

            inline Resource& get_resource(uint32_t index){
                return resources[index];
            }
            inline Resource* ref_resource(uint32_t index){
                return &resources[index];
            }

            inline Resource& get_last_added_resource(){
                return resources[last_added];
            }

            inline Resource* ref_last_added_resource(){
                return &resources[last_added];
            }

            // Remove a resource
            void remove_resource(const std::string& name) {
                auto it = resourceMap.find(name);
                if (it != resourceMap.end() [[likely]]) {
                    size_t index = it->second;
                    resourceMap.erase(it); 
                    freeIndices.push(index); // Mark index as free
                }
            }

            // Access all resources
            std::span<Resource> get_all_resources() {
                return std::span<Resource>(resources.data(), resources.size());
            }

            // Get the count of all resources
            size_t getCount() const {
                return resources.size();
            }

            // Clear all resources
            void clearResources() {
                resources.clear();
                resourceMap.clear();
                std::queue<size_t> empty;
                std::swap(freeIndices, empty);
            }
        };
    }
}