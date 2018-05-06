# Computer-Vision-Class
Homework of computer vision class in sjtu

## Asignment 1: Morphological
### Implementation of Fundamental Morphological Operations
Write your own implementations of the morphological dilation and erosion operations. Your programs should input a binary image (see attached 'lena-binary.bmp' file as a matrix) and a structuring element (also as a matrix), and produce a binary image (another matrix) as the result of the operation. 

You can generate the structuring element as a binary image with an arbitrary shape or use a predefined structure (such as a square or a disc) with a user-defined parameter for its size (such as the length of the side of the square or the diameter of the disc). Given the structuring element, your code should implement the dilation and erosion operations using the definitions given in the course slides. Note that the structuring element is created outside and given as an input to the dilation/erosion codes so that these codes can work with any kind of structuring element. You are free to use any programming language.

## Asignment 2: Binaryzation
### Implementation of Otsu's Method
Write your own implementation of Otsu's method. Your programs should input a grey level image (see attached 'cherry.png' file as a matrix), and produce your optimal threshold values and two output images (foreground and background).

## Asignment 3: Filter
### Implementation of spatial domain filters
Write your own implementation of spatial domain filterings. Your programs should input a grey level image (see attached 'filters.png' file as a matrix), and produce output images processed by spatial domain filtering. You will need to use Gaussian, Mean and Median as filter to convolve the input image.

## Asignment 4: Enhancement
### Histogram Equalization Practice
You are required to apply Histogram Equalization (HE) algorithm to process the attached under-exposed image as input image data to make it visually easier for human recognition. In addition to the final processed image, histograms of both the original and output images should be included. You are also required to write a simple document to explain HE algorithm itself and your own implementation.

## Asignment 5: Fourier
### Fourier Transform Practice
You are required to write an application to implement and understand Fourier Series and Fourier Transform for image processing. 

Please use both trigonometric and exponential functions of Fourier transform to create Fourier Spectrum and Fourier Phase Spectrum. With these information, your application can apply Fourier Transform and Inverse Fourier Transform. Your application is capable of selecting and selecting certain ranges of frequencies of an image. By restoring the remains of the Fourier data, your application is capable of removing details or outlook of an image.

## Asignment 6: SIFT
### Try with SIFT / SUF
Apply SIFT or SURF class in OpenCV 

> http://docs.opencv.org/2.4/modules/nonfree/doc/feature_detection.html#sift

Extract features in photos taken by yourself, the image may be of different orientation, illumination and scale. Tr to match the two images with the use of SIFT/SURF descriptor.


# Final Project