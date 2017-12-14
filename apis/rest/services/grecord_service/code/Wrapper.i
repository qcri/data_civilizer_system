/* File : Wrapper.i */
%module Wrapper

%{
#include "Wrapper.h"
%}

/* Let's just grab the original header file here */
%include "Wrapper.h"
