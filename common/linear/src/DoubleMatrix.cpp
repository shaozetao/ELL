// DoubleMatrix.cpp

#include "DoubleMatrix.h"
#include <cassert>

using std::runtime_error;
using std::fill;

namespace linear
{
	uint DoubleMatrixBase::NumRows() const
	{
		return _num_rows;
	}

	uint DoubleMatrixBase::NumColumns() const
	{
		return _num_columns;
	}

	void DoubleMatrixBase::set(uint i, uint j, double value)
	{
		operator()(i,j) = value;
	}

	void DoubleMatrixBase::Reset()
	{
		fill(begin(), end(), 0);
	}

	void DoubleMatrixBase::Gemv(const double* p_x, double* p_y, double alpha, double beta) const
	{
		int size = (int)NumRows(); // openmp doesn't like uint

		if(alpha == 1.0)
		{
			if (beta == 0.0) // alpha == 1.0 && beta == 0.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] = (double)RowDot(i, p_x);
				}
			}
			else if (beta == 1.0) // alpha == 1.0 && beta == 1.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] += (double)RowDot(i, p_x);
				}
			}
			else // alpha == 1.0 && beta != 0.0 && beta != 1.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] = (double)(RowDot(i, p_x) + p_y[i] * beta);
				}
			}
		}
		else 
		{
			if (beta == 0.0) // alpha != 1.0 && beta == 0.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] = (double)(RowDot(i, p_x) * alpha);
				}
			}
			else if (beta == 1.0) // alpha != 1.0 && beta == 1.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] += (double)(RowDot(i, p_x) * alpha);
				}
			}
			else // alpha != 1.0 && beta != 0.0 && beta != 1.0
			{
                #pragma omp parallel for
				for (int i = 0; i < size; ++i)
				{
					p_y[i] = (double)(RowDot(i, p_x) * alpha + p_y[i] * beta);
				}
			}
		}
	}

	void DoubleMatrixBase::Gevm(const double* p_x, double* p_y, double alpha, double beta) const
	{
		int size = (int)NumColumns(); // openmp doesn't like uint

		if (alpha == 1.0)
		{
			if (beta == 0.0) // alpha == 1.0 && beta == 0.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] = (double)ColumnDot(j, p_x);
				}
			}
			else if(beta == 1.0) // alpha == 1.0 && beta == 1.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] += (double)ColumnDot(j, p_x);
				}
			}
			else // alpha == 1.0 && beta != 0.0 && beta != 1.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] = (double)(ColumnDot(j, p_x) + p_y[j] * beta);
				}
			}
		}
		else 
		{
			if (beta == 0.0) // alpha != 1.0 && beta == 0.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] = (double)(ColumnDot(j, p_x) * alpha);
				}
			}
			else if (beta == 1.0) // alpha != 1.0 && beta == 1.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] += (double)(ColumnDot(j, p_x) * alpha);
				}
			}
			else // alpha != 1.0 && beta != 0.0 && beta != 1.0
			{
                #pragma omp parallel for
				for (int j = 0; j < size; ++j)
				{
					p_y[j] = (double)(ColumnDot(j, p_x) * alpha + p_y[j] * beta);
				}
			}
		}
	}

	void DoubleMatrixBase::print(ostream& os) const
	{
		for(uint i = 0; i < _num_rows; ++i)
		{
			for(uint j = 0; j < _num_columns; ++j)
			{
				os << this->operator()(i, j) << "\t";
			}
			os << std::endl;
		}
	}

	DoubleMatrixBase::DoubleMatrixBase(uint size, uint NumRows, uint NumColumns)
		: vector<double>(size), _num_rows(NumRows), _num_columns(NumColumns)
	{}

	double DoubleMatrixBase::RowDot(uint i, const double* p_x) const
	{
		double value = 0;
		for (uint j = 0; j < NumColumns(); ++j)
		{
			value += (double)(this->operator()(i, j) * p_x[j]);
		}
		return value;
	}

	double DoubleMatrixBase::ColumnDot(uint j, const double* p_x) const
	{
		double value = 0;
		for (uint i = 0; i < NumRows(); ++i)
		{
			value += (double)(this->operator()(i, j) * p_x[i]);
		}
		return value;
	}

	// MatrixStructure::Column

	DoubleMatrix<MatrixStructure::Column>::DoubleMatrix(uint NumRows, uint NumColumns) :
		DoubleMatrixBase(NumRows*NumColumns, NumRows, NumColumns)
	{}

	double& DoubleMatrix<MatrixStructure::Column>::operator()(uint i, uint j)
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		return this->operator[](j * this->_num_rows + i);
	}

	double DoubleMatrix<MatrixStructure::Column>::operator()(uint i, uint j) const
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		return this->operator[](j * this->_num_rows + i);
	}

	// MatrixStructure::Row

	DoubleMatrix<MatrixStructure::Row>::DoubleMatrix(uint NumRows, uint NumColumns) :
		DoubleMatrixBase(NumRows*NumColumns, NumRows, NumColumns)
	{}

	double& DoubleMatrix<MatrixStructure::Row>::operator()(uint i, uint j)
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		return this->operator[](i * this->_num_columns + j);
	}

	double DoubleMatrix<MatrixStructure::Row>::operator()(uint i, uint j) const
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		return this->operator[](i * this->_num_columns + j);
	}

	// MatrixStructure::column_square

	DoubleMatrix<MatrixStructure::column_square>::DoubleMatrix(uint dimension) :
		DoubleMatrix<MatrixStructure::Column>(dimension, dimension)
	{}

	// MatrixStructure::row_square

	DoubleMatrix<MatrixStructure::row_square>::DoubleMatrix(uint dimension) :
		DoubleMatrix<MatrixStructure::Row>(dimension, dimension)
	{}

	// MatrixStructure::row_square_uptriangular

	DoubleMatrix<MatrixStructure::row_square_uptriangular>::DoubleMatrix(uint dimension) :
		DoubleMatrixBase(dimension*(dimension + 1) / 2, dimension, dimension)
	{}

	void DoubleMatrix<MatrixStructure::row_square_uptriangular>::set(uint i, uint j, double value)
	{
		if(i > j)
		{
			throw runtime_error("Cannot set element in the lower triangle of an upper triangular matrix");
		}

		DoubleMatrixBase::set(i, j, value);
	}

	double& DoubleMatrix<MatrixStructure::row_square_uptriangular>::operator()(uint i, uint j)
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		_dummy = 0;
		if(i > j) return _dummy;
		return this->operator[](i * this->_num_columns - (i + 1) * i / 2 + j);
	}

	double DoubleMatrix<MatrixStructure::row_square_uptriangular>::operator()(uint i, uint j) const
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		if(i > j) return 0;
		return this->operator[](i * this->_num_columns - (i + 1) * i / 2 + j);
	}

	void DoubleMatrix<MatrixStructure::row_square_uptriangular>::Gemv(const double* p_x, double* p_y, double alpha, double beta) const
	{
		// TODO - replace the line below with optimized code for this type of matrix
		DoubleMatrixBase::Gemv(p_x, p_y, alpha, beta);
	}

	void DoubleMatrix<MatrixStructure::row_square_uptriangular>::Gevm(const double* p_x, double* p_y, double alpha, double beta) const
	{
		// TODO - replace the line below with optimized code for this type of matrix
		DoubleMatrixBase::Gevm(p_x, p_y, alpha, beta);
	}

	// MatrixStructure::diagonal

	DoubleMatrix<MatrixStructure::diagonal>::DoubleMatrix(uint dimension) :
		DoubleMatrixBase(dimension, dimension, dimension)
	{}

	void DoubleMatrix<MatrixStructure::diagonal>::set(uint i, uint j, double value)
	{
		if(i != j)
		{
			throw runtime_error("Cannot set off-diagonal elements of a diagonal matrix");
		}

		DoubleMatrixBase::set(i, j, value);
	}

	double& DoubleMatrix<MatrixStructure::diagonal>::operator()(uint i, uint j)
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		_dummy = 0;
		if(i != j) return _dummy;
		return this->operator[](i);
	}

	double DoubleMatrix<MatrixStructure::diagonal>::operator()(uint i, uint j) const
	{
		assert(i < this->_num_rows && j < this->_num_columns);
		if(i != j) return 0;
		return this->operator[](i);
	}

	void DoubleMatrix<MatrixStructure::diagonal>::Gemv(const double* p_x, double* p_y, double alpha, double beta) const
	{
		// TODO - replace the line below with optimized code for this type of matrix
		DoubleMatrixBase::Gemv(p_x, p_y, alpha, beta);
	}

	void DoubleMatrix<MatrixStructure::diagonal>::Gevm(const double* p_x, double* p_y, double alpha, double beta) const
	{		
		// TODO - replace the line below with optimized code for this type of matrix
		DoubleMatrixBase::Gevm(p_x, p_y, alpha, beta);
	}
}