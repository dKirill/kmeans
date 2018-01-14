//
//  kmeans.hpp
//  kmeans
//
//  Created by Kirill Delimbetov on 13.01.2018.
//

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

#define OUT(x) std::cout << x << '\n';
#define OUTELEM(x) {for(const auto& e : x) std::cout << e << ' '; std::cout << '\n';}

namespace clustering {
	
	/// Dim is number of dimensions
	template<size_t Dim>
	class VectorSpace {
	public:
		// MARK: Nested types
		using BaseType = float;
		using Element = std::array<BaseType, Dim>;
		using ElementsBatch = std::vector<Element>; // complete locality
		using Norm = BaseType(*)(const Element&, const Element&); // dont use std::function because max perf is needed for this func
		using RandomGenerator = std::function<BaseType()>;
		
		/// criteria to stop algorithm
		struct TerminationCriteria {
			BaseType epsilon;
			int32_t maxNumberOfIterations;
		};
		
		// MARK: Supported norms
		static inline BaseType l1norm(const Element& e1, const Element& e2) {
			BaseType res = 0;
			
			for(auto idx = decltype(Dim){0}; idx < Dim; ++idx) {
				res += std::fabs(e2[idx] - e1[idx]);
			}
			
			return res;
		}
		
		static inline BaseType l2norm(const Element& e1, const Element& e2) {
			BaseType res = 0;
			
			for(auto idx = decltype(Dim){0}; idx < Dim; ++idx) {
				const auto diff = e2[idx] - e1[idx];
				
				res += diff * diff;
			}
			
			return std::sqrt(res);
		}
		
		// MARK: Clustering algorithms
		/// @tparam distance is norm function to use (is crucial to performance of algorithm so making it template param is an attempt to maximize inlining chances)
		/// @param elements subset of vectors to cluster.
		/// @param termCriteria criteria to end algorithm.
		/// @param [in, out] generator function that generates uniform random numbers
		/// @param [in, out] centerIdxs vector of cluster centers where index is cluster index and value is center. Size is desired number of clusters
		/// @param [out] elementToClusterMap vector that maps every element to its cluster. Index is index of @ref elements and value is id of cluster its element is appointed to (index of @ref centers). Must be of the same size as @ref elements.
		/// @return true on success, false on failure
		template<Norm distance>
		static bool kmeans(
						   const ElementsBatch& elements,
						   const TerminationCriteria& termCriteria,
						   RandomGenerator& generator,
						   std::vector<Element>& centers,
						   std::vector<typename std::vector<Element>::size_type>& elementToClusterMap) {
			// MARK: check input correctness
			{
				static_assert(Dim > 0, "There should be more than 0 dimensions");
				
				if(elements.empty()) {
					OUT("elements is empty");
					return false;
				}
				
				if(termCriteria.maxNumberOfIterations > 1 && termCriteria.epsilon <= 0) {
					OUT("termination criteria is incorrect");
					return false;
				}
				
				if(centers.size() < 1) {
					OUT("number of clusters is less than 1");
					return false;
				}
				
				if(elementToClusterMap.size() != elements.size()) {
					OUT("elementToClusterMap must be of the same size as elements");
					return false;
				}
			}
			
			// MARK: kmeans++ initialization
			{
				// using vector because stack might be overflowed for this size of data
				// this var will be used as squared distance container for kmeans++
				std::vector<double> sqdistances(elements.size());
				// 1. choose first center randomly from every element
				const auto firstCenter = static_cast<size_t>(generator() * BaseType(elements.size()));
				
				centers[0] = elements[firstCenter];
				
				for(size_t cidx = 1; cidx < centers.size(); ++cidx) {
					// 2. for every point calc its squared distance to closest already chosen centroid and sum of these squares
					for(size_t eidx = 0; eidx < elements.size(); ++eidx) {
						// calc distance to every chosen centroid and pick shortest
						auto shortestDistSq = distance(elements[eidx], centers[0]);
						
						// square it
						shortestDistSq *= shortestDistSq;
						
						for(size_t chosenIdx = 1; chosenIdx < cidx; ++chosenIdx) {
							const auto dist = distance(elements[eidx], centers[chosenIdx]);
							
							shortestDistSq = std::min(dist * dist, shortestDistSq);
						}
						
						// add picked dist to sum
						sqdistances[eidx] = shortestDistSq + (eidx > 0 ? sqdistances[eidx - 1] : 0);
					}
					
					// 3. pick next centroid using d^2 weighting
					const auto random = generator() * sqdistances.back();
					const auto firstOverRandom = std::upper_bound(sqdistances.begin(), sqdistances.end(), random);
					const auto firstOverRandomIdx = std::distance(sqdistances.begin(), firstOverRandom);
					
					centers[cidx] = elements[size_t(firstOverRandomIdx)];
				}
			}
			
			// MARK: kmeans clustering
			{
				auto iterNum = int32_t{0};
				std::vector<std::pair<Element, int32_t>> clusterAvgAndCountOfElements(centers.size());
				
				for( ; ; ) {
					auto maxCenterMoveDistance = std::numeric_limits<BaseType>::min();
					
					// reset
					for(auto& pair : clusterAvgAndCountOfElements)
						pair.second = 0;
					
					// MARK: assign every point to closest centroid
					for(size_t eidx = 0; eidx < elements.size(); ++eidx) {
						// calc distance to every centroid and pick shortest
						auto shortestDist = std::numeric_limits<BaseType>::max();
						auto closestCenterIdx = size_t{0};
						
						for(size_t cidx = 0; cidx < centers.size(); ++cidx) {
							const auto dist = distance(elements[eidx], centers[cidx]);
							
							// remember closest centroid
							if(dist < shortestDist) {
								shortestDist = dist;
								closestCenterIdx = cidx;
							}
						}
						
						// assign to closest centroid
						elementToClusterMap[eidx] = closestCenterIdx;
						
						// adjust centroid
						auto weighted = elements[eidx];
						
						++clusterAvgAndCountOfElements[closestCenterIdx].second;
						mul(weighted, BaseType{1} / clusterAvgAndCountOfElements[closestCenterIdx].second);
						mul(clusterAvgAndCountOfElements[closestCenterIdx].first, BaseType(clusterAvgAndCountOfElements[closestCenterIdx].second - 1) / clusterAvgAndCountOfElements[closestCenterIdx].second);
						add(clusterAvgAndCountOfElements[closestCenterIdx].first, weighted);
					}
					
					// MARK: recalculate centroids
					for(size_t cidx = 0; cidx < centers.size(); ++cidx) {
						const auto dist = distance(centers[cidx], clusterAvgAndCountOfElements[cidx].first);
						maxCenterMoveDistance = std::max(dist, maxCenterMoveDistance);
						centers[cidx] = clusterAvgAndCountOfElements[cidx].first;
					}
					
					// MARK: check for completeness
					if(++iterNum >= termCriteria.maxNumberOfIterations) {
						OUT("Iteration num ceil is hit");
						break;
					}
					
					if(maxCenterMoveDistance < termCriteria.epsilon) {
						OUT("Centers change less than epsilon");
						break;
					}
				}
			}
			
			return true;
		}
		
	private:
		static inline void mul(Element& elem, const BaseType val) {
			for(auto& e : elem)
				e *= val;
		}
		
		static inline void add(Element& elem1, const Element& elem2) {
			for(auto idx = size_t{0}; idx < elem1.size(); ++idx)
				elem1[idx] += elem2[idx];
		}
	};
	
} // clustering
