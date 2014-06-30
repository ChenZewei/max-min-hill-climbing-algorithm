#include <omp.h>
#include <R.h>
#include <Rmath.h>
#include <vector>
#include <tr1/unordered_map>
#include <RcppArmadillo.h>
#include <time.h>
#include <string>
#include <sstream>
using namespace std;
using namespace std::tr1;
using namespace Rcpp;
using namespace arma;

// [[Rcpp::depends("RcppArmadillo")]]
// [[Rcpp::plugins(openmp)]]

// int getN_ij();
// int getSingleN_ijk(int*, int*, int);
// double ScoreEmptyGraph(SEXP, int*, int*, double*, int*);
// double BDeu();

// int main (int argc, char *argv[]) {

// 	return 0;
// }

// [[Rcpp::export]]
int getR(NumericVector& f) {
	unordered_map<int, int> Map;

	for (NumericVector::iterator it = f.begin(); it != f.end(); it++) {
		Map[*it] = 1;
	}

	return Map.size();
}

string int_array_to_string(NumericVector& array) { //  int int_array[], int size_of_array){
	ostringstream oss("");
	for (int temp = 0; temp < array.size(); temp++)
		oss << array[temp];
	
	return oss.str();
}

//Faster!!!!!!!!!!!!!!!!!!!
// // [[Rcpp::export]]
// void PrintC(SEXP x, SEXP N, SEXP M) {
// 	double *A = REAL(x);
// 	int *n = INTEGER(N), *m = INTEGER(M);

// 	for (int i = 0; i < *m; i++)
// 	{
// 		for (int j = 0; j < *n; j++)
// 		{
// 			// cout << A[i + j * (*m)] << " ";
// 			A[i + j * (*m)] = sqrt(2);
// 		}
// 		// cout << endl;
// 	}
// }

// // [[Rcpp::export]]
// void PrintCpp(SEXP x, SEXP N, SEXP M) {
// 	double *A = REAL(x);
// 	int *n = INTEGER(N), *m = INTEGER(M);

// 	for (int i = 0; i < *n; i++)
// 	{
// 		for (int j = 0; j < *m; j++)
// 		{
// 			// cout << A[i + j * (*n)] << " ";
// 			A[i + j * (*n)] = sqrt(2);
// 		}
// 		// cout << endl;
// 	}
// }

// [[Rcpp::export]]
unordered_map<int, string> UniqueMap(SEXP x) {
	NumericMatrix A(x);
	NumericVector u;
	string key;
	unordered_map<string, int> Map;
	unordered_map<int, string> Uni;
	int n = Map.size(), k = 0;

	for (int i = 0; i < A.nrow(); i++)
	{
		u = A(i, _);
		key = int_array_to_string(u);
		Map[key] = 1;
		if (Map.size() != n) {
			Uni[k] = key;
			n++;
			k++;
		}
	}

	return Uni;
}

int getSingleN_ijk(double *vec, int dim, int k) {
	int count = 0;

	for (int i = 0; i < dim; i++)
	{
		if (vec[i] == k)
			count++;
	}

	return count;
}

int getVecN_ijk(double *vec, double *parentVec, int dim, int j, int k) {
	int count = 0;

	for (int i = 0; i < dim; i++)
	{
		if (vec[i] == k && parentVec[i] == j)
			count++;
	}

	return count;
}

int getMapN_ijk(double *vec, NumericMatrix& parentMatrix, unordered_map<int, string> parentMap, int dim, int j, int k) {
	int count = 0;
	NumericVector u;

	for (int i = 0; i < dim; i++)
	{
		u = parentMatrix(i, _);
		if (vec[i] == k && parentMap[j-1] == int_array_to_string(u))
			count++;
	}

	return count;
}

// [[Rcpp::export]]
double ScoreNodeWithNoneParents(SEXP column, SEXP N, int r, double eta) {
	int *n = INTEGER(N), dim = XLENGTH(column);
	double *vec = REAL(column);
	double gammaJ = eta / 1.0, gammaK = eta / (1.0 * r);
	double rScore = 0.0, qScore = 0.0;
	int n_ijk = 0, n_ij = 0;

	for (int k = 1; k <= r; k++)
	{
		n_ijk = getSingleN_ijk(vec, dim, k);
		n_ij += n_ijk;
		rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
	}

	qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;


	return qScore;
}

// [[Rcpp::export]]
double ScoreNodeWithOneParent(SEXP Xi, SEXP Pa, SEXP N, int r, int q, double eta) {
	int *n = INTEGER(N), dim = XLENGTH(Xi);
	double *vec = REAL(Xi), *parentVec = REAL(Pa);
	double gammaJ = eta / q, gammaK = eta / (q * r);
	double rScore = 0.0, qScore = 0.0;
	int n_ij, n_ijk = 0;


	for (int j = 1; j <= q; j++)
	{
		n_ij = 0;
		rScore = 0.0;

		for (int k = 1; k <= r; k++)
		{
			n_ij += getVecN_ijk(vec, parentVec, dim, j, k);
		}

		for (int k = 1; k <= r; k++)
		{
			n_ijk = getVecN_ijk(vec, parentVec, dim, j, k);
			rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
		}

		qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;
	}


	return qScore;
}

// [[Rcpp::export]]
double ScoreNodeWithMoreParents(SEXP Xi, SEXP Pa, SEXP N, SEXP R, SEXP Q, SEXP Eta) {
	int *n = INTEGER(N), *r = INTEGER(R), *q = INTEGER(Q), dim = XLENGTH(Xi);
	double *vec = REAL(Xi), *eta = REAL(Eta);
	NumericMatrix Parent(Pa);
	unordered_map<int, string> parentMap = UniqueMap(Parent);
	double gammaJ = *eta / (*q), gammaK = *eta / ((*q) * (*r));
	double rScore = 0.0, qScore = 0.0;
	int n_ij, n_ijk = 0;

	for (int j = 1; j <= *q; j++)
	{
		n_ij = 0;
		rScore = 0.0;

		for (int k = 1; k <= *r; k++)
		{
			n_ij += getMapN_ijk(vec, Parent, parentMap, dim, j, k);
		}

		for (int k = 1; k <= *r; k++)
		{
			n_ijk = getMapN_ijk(vec, Parent, parentMap, dim, j, k);
			rScore += lgamma( n_ijk + gammaK ) - lgamma( gammaK );
		}

		qScore += lgamma( gammaJ ) - lgamma( n_ij + gammaJ ) + rScore;
	}


	return qScore;
}

NumericVector InitScore(NumericMatrix& A, SEXP N, NumericVector& R, double eta) {
	int *n = INTEGER(N);
	NumericVector g, score(*n);

	for (int i = 0; i < *n; i++) {
		g = A(_, i);
		score[i] = ScoreNodeWithNoneParents(g, N, R[i], eta);
	}

	return score;
}

// [[Rcpp::export]]
SEXP BDeu(SEXP x, SEXP y, SEXP z, SEXP N) {
	NumericVector g, f, R(y), scores;
	unordered_map<int, int> Node;
	int *n= INTEGER(N), q;
	NumericMatrix A(x), AdjMat(*n, *n);
	double addScore, reverseScore, eta = 1.0;
	List PC(z);

	scores = InitScore(A, N, R, eta);

	for (int i = 0; i < PC.size(); i++)
	{
		NumericVector pc = as<NumericVector>(PC[i]);
		if (pc.size() == 1) {
			q = R[pc[0] - 1];
			g = A(_, i);
			f = A(_, (pc[0]-1));
			addScore = ScoreNodeWithOneParent(g, f, N, R[i], q, eta);
			g = A(_, (pc[0]-1));
			f = A(_, i);
			reverseScore = ScoreNodeWithOneParent(g, f, N, R[i], q, eta);
			if (addScore > scores[i]) {
				scores[i] = addScore;
				AdjMat(i, (pc[0]-1)) = 1;
			}
		}
	}
	
	// while (highestScore < currentScore) {

	// 	highestScore = currentScore;


	// }
	return AdjMat;
}