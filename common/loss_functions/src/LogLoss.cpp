// LogLoss.cpp

#include "LogLoss.h"

#include <cmath>
using std::log;
using std::exp;

namespace loss_functions
{
	LogLoss::LogLoss(double Scale) : _scale(Scale)
	{}

	double LogLoss::Evaluate(double prediction, double label) const
	{
		double scaled_margin = _scale * prediction * label;

		if(scaled_margin <= -18.0)
		{
			return -prediction * label;
		}
		else 
		{
			double exp_neg_scaled_margin = exp(-scaled_margin);
			return log(1.0 + exp_neg_scaled_margin) / _scale;
		}

		return 0.0;
	}

	double LogLoss::GetDerivative(double prediction, double label) const
	{
		double scaled_margin = prediction * label * _scale;

		if(scaled_margin <= 0.0)
		{
			double exp_scaled_margin = exp(scaled_margin);
			return -label / (1.0 + exp_scaled_margin);
		}
		else 
		{
			double exp_neg_scaled_margin = exp(-scaled_margin);
			return -label * exp_neg_scaled_margin / (1.0 + exp_neg_scaled_margin);
		}
	}
}
