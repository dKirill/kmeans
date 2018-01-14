//
//  Test.cpp
//  tests
//
//  Created by Kirill Delimbetov on 13.01.2018.
//

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <catch.hpp>
#include <kmeans.hpp>

using namespace clustering;

// MARK: Settings
struct Constant {
	static constexpr auto dimNum = 20;
	static constexpr auto batchSize = 10003;
	static constexpr auto centerCount = 10;
	static constexpr auto benchRuns = 10;
};

template<class FloatType>
static bool fequal(const FloatType& f1, const FloatType& f2) {
	return std::fabs(f2 - f1) < std::numeric_limits<FloatType>::epsilon();
}

TEST_CASE("L1 norm") {
	using VSpace10 = VectorSpace<10>;
	VSpace10::Element elem1;
	VSpace10::Element elem2;
	
	// fill elements
	std::iota(elem1.begin(), elem1.end(), 0);
	std::iota(elem2.rbegin(), elem2.rend(), 0);
	
	const auto manhattanDistance = VSpace10::l1norm(elem1, elem2);
	
	REQUIRE(fequal(manhattanDistance, 50.f));
}

TEST_CASE("L2 norm") {
	using VSpace10 = VectorSpace<10>;
	VSpace10::Element elem1;
	VSpace10::Element elem2;
	
	// fill elements
	std::iota(elem1.begin(), elem1.end(), 0);
	std::iota(elem2.rbegin(), elem2.rend(), 0);
	
	const auto euclideanDistance = VSpace10::l2norm(elem1, elem2);
	
	REQUIRE(fequal(euclideanDistance, 18.165902f));
}

template<VectorSpace<Constant::dimNum>::Norm norm>
void kmeansTest() {
	using VSpace = VectorSpace<Constant::dimNum>;
	VSpace::ElementsBatch elements(Constant::batchSize);
	VSpace::TerminationCriteria termCriteria = { 0.1f, 10000 };
	std::vector<VSpace::Element> centers(Constant::centerCount);
	std::vector<VSpace::ElementsBatch::size_type> elementToClusterMap(elements.size());
	// init random generator
	std::default_random_engine rengine;
	std::uniform_real_distribution<VSpace::BaseType> distribution(0.0, 1.0);
	VSpace::RandomGenerator generator = [&rengine, &distribution] { return distribution(rengine); };
	ctpl::thread_pool tpool(std::thread::hardware_concurrency());
	
	// generate random data but with very explicit clusters so its easy to test
	int num = 0;
	const auto step = elements.size() / centers.size();
	size_t idx = 0;
	
	for(auto& elem : elements) {
		std::generate(elem.begin(), elem.end(), [num, &generator] {
			return 100 * num + generator();
		});
		
		// every step element start new cluster
		if(++idx % step == 0)
			++num;
	}
	
	// do clustering
	REQUIRE(VSpace::kmeans<norm>(elements, termCriteria, tpool, generator, centers, elementToClusterMap));
	
	// check that every cluster center is unique
	REQUIRE(std::unique(centers.begin(), centers.end()) == centers.end());
	
	// check that every step elements are in the same cluster
	for(size_t idx = 0; idx < elementToClusterMap.size(); idx += step) {
		const auto cluster = elementToClusterMap[idx];
		
		for(size_t jdx = 1; jdx < step && idx + jdx < elementToClusterMap.size(); ++jdx) {
			REQUIRE(elementToClusterMap[idx + jdx] == cluster);
		}
	}
}

TEST_CASE("kmeans l1") {
	using VSpace = VectorSpace<Constant::dimNum>;
	kmeansTest<VSpace::l1norm>();
}

TEST_CASE("kmeans l2") {
	using VSpace = VectorSpace<Constant::dimNum>;
	kmeansTest<VSpace::l2norm>();
}


template<VectorSpace<Constant::dimNum>::Norm norm>
void kmeansBench() {
	using VSpace = VectorSpace<Constant::dimNum>;
	VSpace::ElementsBatch elements(Constant::batchSize);
	VSpace::TerminationCriteria termCriteria = { 0.1f, 10000 };
	std::vector<VSpace::Element> centers(Constant::centerCount);
	std::vector<VSpace::ElementsBatch::size_type> elementToClusterMap(elements.size());
	// init random generator
	std::default_random_engine rengine;
	std::uniform_real_distribution<VSpace::BaseType> distribution(0.0,1.0);
	VSpace::RandomGenerator generator = [&rengine, &distribution] { return distribution(rengine); };
	ctpl::thread_pool tpool(std::thread::hardware_concurrency());
	
	// generate random data
	for(auto& elem : elements) {
		std::generate(elem.begin(), elem.end(), [&generator] {
			return 1000 * generator();
		});
	}
	
	// measure
	auto t1 = std::chrono::high_resolution_clock::now();
	
	for(int i = 0; i < Constant::benchRuns; ++i) {
		// do clustering
		REQUIRE(VSpace::kmeans<norm>(elements, termCriteria, tpool, generator, centers, elementToClusterMap));
	}
	
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - t1 ).count();
	OUT(__PRETTY_FUNCTION__ << " takes " << (duration / Constant::benchRuns)  << "ms per run\n");
}

TEST_CASE("bench l1") {
	using VSpace = VectorSpace<Constant::dimNum>;
	kmeansBench<VSpace::l1norm>();
}

TEST_CASE("bench l2") {
	using VSpace = VectorSpace<Constant::dimNum>;
	kmeansBench<VSpace::l2norm>();
}
