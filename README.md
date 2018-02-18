# OpenGL-2D-LaserShootingGame

- ### Demo

  ![Laser Shooting Demo](https://github.com/{user}/{repo}/demo.gif)

- ### Dependencies

  - #### Recommended
  
    - **GLFW**
      - Install **glfw** in Linux (Debian based) `sudo apt install libglfw3 libglfw3-dev`

    - **GLAD**
      - Go to [glad link](http://glad.dav1d.de/)
      - **`Language`** : **`C/C++`**
      - **`Specification`** : **`OpenGL`**
      - **`gl`** : **`Version 4.5`**
      - **`gles1`** : **`Version 1.0`**
      - **`gles2`** : **`Version 3.2`**
      - **`Profile`** : **`Core`**
      - Select `Add All` under extensions and click Generate and Download the zip file generated.
      - Copy contents of `include/` folder in the downloaded directory to `/usr/local/include/`
      - `src/glad.c` should be always compiled along with your OpenGL code
  
    - **GLM**
      - Download the zip file from [glm link](https://github.com/g-truc/glm/releases/tag/0.9.7.2)
      - Unzip it and copy the folder `glm/glm/` to `/usr/local/include`
    

    - **GLUT**
      - Install **glut** in `Linux` (Debian based) `sudo apt install freeglut3`
    
    - **GLEW**
      - Install **glew** in `Linux` (Debian based) `sudo apt install libglew-dev` 
    
  - It is recommended to use GLFW+GLAD+GLM on all OSes.
  - Ubuntu users can also install these libraries using apt-get.
    
- ### Instructions

  - **Compile**
    - generate executable using makefile `make`
    
  - **Run**
    - execute `Sample2D`
    
  - **Controls**
    - **`S`** move canon **`UP`**
    - **`F`** move canon **`DOWN`**
    - **`space`** canon **`SHOOT`**
    - **`A`** rotate canon **`ANTI CLOCKWISE`**
    - **`D`** rotate canon **`CLOCKWISE`**
    - **`LEFT CTRL`** move green bucket **`LEFT`**
    - **`RIGHT CTRL`** move green bucket **`RIGHT`**
    - **`LEFT ALT`** move red bucket **`LEFT`**
    - **`RIGHT ALT`** move green bucket **`RIGHT`**
