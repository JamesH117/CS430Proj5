# CS430Proj5
Project 5 - Image Viewer

Image Viewer that should open up an input PPM image and display it in a GLFW window.
The GLFW window will allow the user to translate, rotate, scale, and shear the image.

Compile the C file using nmake.
Run the program by calling: ezview input.ppm
where input.ppm is the name of the ppm image being provided to the program.

##Controls
A & D : Roates the image by 90 degree increments.
W & S: Scale the image by 30% increments.
Z & X: Shear the image by 30% increments
Q & E: Translate the image in positive or negative X directions.

##Known Problems
Non square images bug out and display incorrectly.
Image types that are converted to PPM and not created as a PPM do not seem to work.  
	If the image is created as a PPM and not converted from another type before hand, it should display correctly, if the image is also a square.