# axiom
principia engine 2

This is a Complete yet not really totally complete rewrite of my Principia Engine
This time it is ECS from the ground up using FLECS. Every Aspect of the engine is using flecs
It also uses namespaces instead of classes
Folder structure is Components/Systems instead of include/src (probably a bad idea but hey lets experiment)
It's also my first official attempt at using cmake which i totally hate and suck at but thanks to chatgpt its some-what bareable now

TODO: Got sponza loading but its not loading in an ecs way cause of needing to properly copy the vulkan buffers
* Next would be to get it textured
* Next would be to implement pbr shaders
* Next... actually probably first would be a proper camera system...