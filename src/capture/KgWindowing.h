#pragma once
#include <vector>
#include <functional>


class KgWindowing
{
public:

	// prototypes
	enum KeType
	{
		k_hamming,          
		k_hann,             
		k_blackman,         // ceoff == 0.42
		k_blackmanharris,   // Blackman-harris (4-term)
		k_blackmanharris7,  // Blackman-harris (7-term)
		k_flattop,          
		k_triangular,       
		k_rectangle,
		k_rcostaper,        // raised-cosine taper
		k_kaiser,          
		k_povey             // KLADI made to be similar to Hamming but to go to zero at the edges
	};

	KgWindowing(unsigned frameSize, KeType type, ...);

	unsigned idim() const { return win_.size(); }
	unsigned odim() const { return win_.size(); }

	void porcess(double* data/*inout*/) const;

	// °ïÖúº¯Êý
	static std::function<double(double)> functor(KeType type, ...);

private:
	std::vector<double> win_;
};

