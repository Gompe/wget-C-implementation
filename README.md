# Tutorial-2a
### Pedro Cabral

To test the program with a simple example, run

> make
> ./wgetX http://info.cern.ch/

Using 

> make
> ./wgetX $url $filename

will save the content returned by wgetX in the file with name $filename.

Use examples:

Gets an image
> ./wgetX http://image.epizeuxis.net/X.gif myfile.gif

Example without redirect
> ./wgetX www.google.com/ myfile.txt 

Example with redirect
> ./whetX google.com/ myfile.txt