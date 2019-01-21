//minimal.i 

%module minimal

%{
#include "minimal.h"
%}

//double-check that this is indeed %include !!!
%include "minimal.h"