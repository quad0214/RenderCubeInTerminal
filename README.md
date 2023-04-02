# RenderCubeInTerminal
This program renders a rotating cube in terminal(command prompt)

![image](https://user-images.githubusercontent.com/34961209/214741189-964b30ac-ac83-4178-b2c0-2d7b42014f22.png)

## Implementation
This program renders a rotation cube using the standard rendering technique.
Convert object coordinates in 6 sides of the cube to screen coordinates. And render(= print specific character) if they are on screen.
For simplification, several assumptions are existed.
- the basis of world, camera space are same.
- position of the cube in world is origin
- etc...

One thing is existed that's different from the standard rendering technique in this program; In the same manner as above, it doesn't progress rasterization. Instead, slices all sides of the cube into small vertices(in program, indicate segments). And converts them to screen space & render. This method doesn't perfect when the cube is close to the camera because it causes holes in rendered sides of the cube. But when the distance between the cube and the camera is enough far, holes are not seen. so I bear(ignore) this.

## Inspired by
https://www.youtube.com/embed/p09i_hoFdd0

## How to use
### Requirements
- windows
- visual studio

If you want to use other environments, download code file(cpp) only and configure makefile.

### Use
- windows & visual studio : clone & open it in visual studio.
- other environment : download code only. and modify it to suit your environment.


## P.S.
### XXX : I THINK THIS PROGRAM HAS A BUG!
If you think this program has a bug or confilct, please issue on this project page.(https://github.com/quad0214/RenderCubeInTerminal/issues)  
Simple requests or questions are also okay.  
Thank you. :)

### And
I'm not good at English. Therefore, incorrect expressions may exist in the document. I apologize in advance for this. If you find a wrong expression, please give me an issue. I'll fix it quickly.  
Thank you. :)
