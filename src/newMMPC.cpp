// #include <RcppCommon.h>
// #include <Rcpp.h>
#include <omp.h>
#include <R.h>
// #include <Rinternals.h>
#include <Rmath.h>
// #include <Rdefines.h>
// #include <vector>
// #include <R_ext/Arith.h>
// #include <R_ext/Utils.h>
// #include <unordered_map>
// #include <tr1/unordered_map>
#include <RcppArmadillo.h>
using namespace std;
using namespace std::tr1;
using namespace Rcpp;
using namespace arma;

// [[Rcpp::depends("RcppArmadillo")]]

// [[Rcpp::export]]
bool allC(SEXP a, SEXP b, int del = -1) {
	bool out = TRUE;
	double *x = REAL(a), *y = REAL(b);
	if(XLENGTH(a)!=XLENGTH(b)) out=FALSE;
	else {
		#pragma omp parallel
		{
			#pragma omp for
			{
				for (int i = 0; i < XLENGTH(a); i++) {
					if(i == del) continue;
					if(del == -2 && XLENGTH(b) == 2) {out=TRUE; break;}
					if(del == -2 && (i == 0 || i == 1)) continue;
					if(x[i]!=y[i]) {
						out=FALSE;
						break;
					}
				}
			}
		}
	}
	return out;
}

// [[Rcpp::export]]
SEXP UN(SEXP x) {
	int *X = INTEGER(x);
	typedef unordered_map<int,int> map_t;
	map_t Map;
	for (int i = 0; i < XLENGTH(x); i++)
	{
		Map.insert(map_t::value_type(X[i], 0));
		Map[X[i]]++;
	}
	return wrap(Map);
}

double combine(double a, double b) {
   	double times = 1;
   	while (times <= b)
		times *= 10;
   	return a*times + b;
} 

// [[Rcpp::export]]
SEXP UNB(SEXP x, SEXP n, SEXP m) {
	double *X = REAL(x);
	int *N = INTEGER(n), *M = INTEGER(m);
	double k;
	typedef unordered_map<int,int> map_t;
	map_t Map;
	for (int i = 0; i < *N; i++)
	{
		k = X[i];
		for (int j = 1; j < *M; j++)
		{
			k = combine(k, X[i + j * (*M)]);
		}
		Map.insert(map_t::value_type(k, 0));
		Map[k]++;
	}
	return wrap(Map);
}

bool IsIn(mat X, mat R) {
	int row = X.n_rows, col = X.n_cols, test;
	bool equaled = FALSE;
	for (int i = 0; i < row; i++)
	{
		test = 0;
		for (int j = 0; j < col; j++)
		{
			if (X(i, j) == R(0, j))
				test++;
		}
		if (test == col) {
			equaled = TRUE;
			break;
		}
	}
	return equaled;
}

void insert(const mat& R, mat& A, int j) {
	for (int i = 0; i < R.n_cols; i++)
	{
		A(j, i) = R(0, i);
	}
}

// [[Rcpp::export]]
SEXP T(SEXP x, SEXP n, SEXP m) {
	double *A = REAL(x);
	int *N = INTEGER(n), *M = INTEGER(m);
	NumericMatrix B(*N, *M);
	for (int i = 0; i < *M; i++)
		for (int j = 0; j < *N; j++)
			B(i, j) = A[j + i * (*M)];
	return B;
}

// // [[Rcpp::export]]
// SEXP Test(SEXP x, SEXP n, SEXP m) {
// 	int *X = INTEGER(x), *N = INTEGER(n), *M = INTEGER(m);
// 	int K = (*N) * (*M);
// 	// int **A = (int**)R_alloc((*N), sizeof(int*));
// 	int *A = (int*)R_alloc(K, sizeof(int));
// 	cout << XLENGTH(A) << endl;
// 	for (int i = 0; i < K; i++)
// 	{
// 		// A[i] = X[i];
// 	}
// 	// for (int i = 0; i < *M; i++)
// 	// {
// 	// 	// A[i] = (int*)R_alloc((*M), sizeof(int));
// 	// 	for (int j = 0; j < *N; j++)
// 	// 	{
// 	// 		cout << X[j + i * (*M)] << endl;
// 	// 		// A[j + i * (*M)] = X[j + i * (*M)];
// 	// 	}
// 	// }
// 	return (SEXP)A;
// }

// // [[Rcpp::export]]
// mat Unique(SEXP x) {
// 	// if (A.n_cols >= A.n_rows) {
// 	// 	continue;
// 	// } else {
// 	// 	x = T(x);
// 	// }




// 	mat U(1, A.n_cols, fill::zeros);
// 	for (int i = 0; i < A.n_rows; i++)
// 	{
// 		if (!(IsIn(U, A.row(i)))) {
// 			insert(A.row(i), U, i);
// 			U.resize(U.n_rows+1, U.n_cols);
// 		}
// 	}
// 	return U;
// }

// // [[Rcpp::export]]
// mat MyTest(mat A) {
// 	return unique(A);
// }

// [[Rcpp::export]]
SEXP UpdateCPC(SEXP x, int selected = 0) {
	NumericVector tmp;
	List cpc(x);
	if (cpc.size() == 0) {
		cpc.push_back(1);
		cpc.push_back(R_NilValue);
	} else if (cpc.size() == 2) {
		cpc[0] = 3;
		cpc.push_back(selected);
	} else {
		int cpcLength = cpc.size();
		cpc[0] = cpcLength + 1;
		cpc.push_back(selected);
		#pragma omp parallel
		{
			#pragma omp for
			{
				for (int i = 2; i < cpcLength; i++)
				{
					tmp = as<NumericVector>(cpc[i]);
					tmp.push_back(selected);
					cpc.push_back(tmp);
				}
			}
		}
	}

	return cpc;
}

// [[Rcpp::export]]
int Df(SEXP x) {
	double *DfSet = REAL(x);
	int df = (DfSet[0]-1)*(DfSet[1]-1);
	#pragma omp parallel
	{
		#pragma omp for
		{
			for (int i = 2; i < XLENGTH(x); i++)
			{
				df*=DfSet[i];
			}
		}
	}
	return df;
}

// [[Rcpp::export]]
NumericVector Statistics(SEXP x, SEXP y, SEXP z) {
	int n, m;
	NumericVector count(4,0.0);
	NumericVector sum(1,0.0);
	NumericVector pvalue(1,0.0);
	NumericVector df(z);
	NumericVector out(2, 0.0);
	
	NumericMatrix A(x);
	NumericMatrix B(y);
	n = A.ncol();
	m = B.ncol();
	NumericVector u;
	NumericVector v;

	#pragma omp parallel
	{
		#pragma omp for collapse(2)
		{
			for (int i = 0; i < m; i++)
			{
				count[0] = count[1] = count[2] = count[3] = 0;
				u = B(_,i);
				for (int j = 0; j < n; j++)
				{
					v = A(_,j);
					if(allC(v,u))
						count[0]++;
					if(allC(v,u,1))
						count[1]++;
					if(allC(v,u,0))
						count[2]++;
					if(allC(v,u,-2))
						count[3]++;
				}
				sum[0] += 2 * count[0] * log( ( count[0] * count[3] ) / ( count[1] * count[2] ) );
			}
		}
	}

	// df = Cardinality(A);
	int DF = Df(df);
	pvalue = pchisq(sum, DF, FALSE);
	out[0] = pvalue[0];
	out[1] = sum[0];

	return out;
}