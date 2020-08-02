#pragma once

/* a simple wrapper for parameters for which there is one value for each channel, e.g. a double or an expression.*/
template <typename type> struct piezoChan{
	type x;
	type y;
	type z;
};
