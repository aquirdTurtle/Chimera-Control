#pragma once

#include <vector>

// a standardized wrapper for experiment data which usually needs a copy of something for each variation. Used to be
// used to handle sequences as well, although sequences were actually working, I just hoped to keep it alive until I 
// didn't.  At this point this is little more than a weird interface to a vector, but could be used in similar spirits
// as before so I'm at least keeping this for now.  - MOB March 20th 2020
template <class type>
class ExpWrap{
	public:
		type& operator() ( unsigned variationNumber ){ 
			if (variationNumber >= data.size ()){
				thrower ("Tried to access experiment wrap structure variation which doesn't exist!");
			}
			return data[ variationNumber ];
		};

		type operator() ( unsigned variationNumber ) const{
			if (variationNumber >= data.size ()){
				thrower ("Tried to access experiment wrap structure variation which doesn't exist!");
			}
			return data[ variationNumber ];
		};

		size_t getNumVariations(  ){
			return data.size();
		}
		void resizeVariations ( unsigned numVariations ){
			data.resize ( numVariations );
		}
		void uniformSizeReset ( unsigned numVariations ){
			data.clear ( );
			data.resize ( numVariations );
		}

	private:
		std::vector<type> data;
};

