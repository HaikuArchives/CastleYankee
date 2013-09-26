Castle Yankee
==============================
Castle Yankee is a programmer's class browser/editor. The idea is to create your C++ classes in a Castle Yankee project, and it will generate the .h and .cpp files, sparing you from having to type the signatures of all member functions and instance variables twice.

### A note on the project files

Castle Yankee project files are a form of XML file that any standard non-validating XML parser should be able to read. However, Castle Yankee doesn't use a real XML parser, and can't accept any arbitrary XML file, even if it were to conform to the correct DTD (supposing for the moment that this DTD actually existed...). Specifically, it requires that the start and end tags begin at the beginning of a line.

Sources obtained from http://www.folta.net/steve/CastleYankee/
"Sources are made available under an MIT-style license."
