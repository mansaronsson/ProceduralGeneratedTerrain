# Procedurally generated terrain
A procedural world made up of chunks. This was our first time implementing an OpenGL project from scratch (*cough* using GLM, GLFW and GLAD). All generation is determenistic, meaning the terrain will always look identical every time it is generated due to the use of Perlin/Simplex noise.
![img](https://imgur.com/a/L872tBB)
---

There are two parts of the project, co-op and solo. The main branch contains the co-op part ([see report for this part here](TSBK03_manar189_anden561.pdf)) where the basic functionality such as basic chunk generation, AABB frustum culling, level of detail and CPU-multithreading. For the second part, we had to split up due to project rules set by the university. Here, we both implemented different biomes with procudurally generated objects. Måns under branch *crystal* and Andreas under branch *ProcuduralTrees*. The following is information about the *crystal* branch (see report for this part here):

## Biomes🌍
Three biomes - plains, desert and crystal mountains - are created, that differ in color and noise functions (vertex y-displacement).

## Crystals💎
Crystals features are varies based on simplex noise functions, with world coodinates as input. Therefore, crystals are both unique and determenistic in both looks and placements.

---

## The devs ☕
This is a course project at Linköping University by the M.Sc. in Media Technology and Engineering students:
- Måns Aronsson ([@mansaronsson](https://github.com/mansaronsson))
- Andreas Engberg ([@pengbergandreas](https://github.com/engbergandreas))
