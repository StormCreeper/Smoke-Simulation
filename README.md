# Smoke simulation using stable fluids and vorticity confinement
_Telo PHILIPPE_

This project builds upon the volumetric cloud renderer I wrote last year ([here](https://github.com/StormCreeper/Volumetric-Cloud-Rendering)), and add a physical simulation of the fluid to try to mimic realistic smoke.

## How to run
- `git clone --recurse-submodule https://github.com/StormCreeper/Smoke-Simulation.git` (make take some time cloning the dependencies)  
- `mkdir Smoke-Simulation/build && cd Smoke-Simulation/build`  
- `cmake ../` (generate the compilation files)  
- `cmake --build . && ./src/Smoke_Simulation.exe` (compile and run the project)

## How to use
There are 3 settings windows:
- Simulation parameters
- Rendering parameters
- Light parameters

You can:
- Rotate the view by scrolling with your track pad, and zoom in an out by scrolling up and down while holding left shift  
- Pause/resume the simulation using the space bar   
- Reset the simulation with the R key  

## How it works
I use the stable fluids method by Jos Stam ([paper](https://www.dgp.toronto.edu/public_user/stam/reality/Research/pdf/ns.pdf)), and vorticity confinement to break the smoothness of the simulation ([explanation here](https://softologyblog.wordpress.com/2019/03/13/vorticity-confinement-for-eulerian-fluid-simulations/))  
The simulation uses an eulerian approach (in opposition to the lagrangian approach which would simulate particles in space), and operates on a 3D texture that contains density and velocity information for the fluid at each voxel.  
At each frame, several compute shaders operate on this texture, to solve numerically each step of the simulation. Finally, the 3D texture is sent to a fragment shader that renders the geometry, and the cloud volume using raymarching.

## Gallery
![image](https://github.com/user-attachments/assets/ed2e5a36-9680-4ebf-936c-db5e101cc080)

![image](https://github.com/user-attachments/assets/9754b750-d1f7-4d6f-b737-fe422711c890)

![image](https://github.com/user-attachments/assets/7a547c99-622e-44ad-a853-497e3858fefb)

![image](https://github.com/user-attachments/assets/791f5018-7858-4888-ac65-b207915c6038)
