# wgetX 
## C implementation of wget linux command

To test the program with a simple example, run

`$ make`
`$ ./wgetX http://info.cern.ch/`

Using 

`$ make`
`$ ./wgetX $url $filename`

will save the content returned by wgetX in the file with name $filename.

Use examples:

Gets an image
`$ ./wgetX http://image.epizeuxis.net/X.gif myfile.gif`

The program also supports redirects. If the http request you make returns a code in 300-399, it will get the redirect url and repeat the http request to this new url. This process will return after a certain number of redirects if no final url is found (to avoid infinite loops). 

Example without redirect
`$ ./wgetX www.google.com/ myfile.txt` 

Example with redirect
`$ ./whetX google.com/ myfile.txt`
