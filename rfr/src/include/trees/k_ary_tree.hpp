#ifndef RFR_K_ARY_TREE_HPP
#define RFR_K_ARY_TREE_HPP

#include<vector>
#include<deque>
#include<algorithm>	// std::shuffle
#include<numeric>	// std::iota
#include<cmath>		// abs


#include "data_containers/data_container_base.hpp"
#include "nodes/temporary_node.hpp"
#include "nodes/k_ary_node.hpp"
#include "trees/tree_base.hpp"
#include "trees/tree_options.hpp"


namespace rfr{

template <const int k,typename split_type, typename RNG_type, typename num_type = float, typename index_type = unsigned int>
class k_ary_random_tree : public rfr::tree_base<num_type, index_type> {
  private:
	std::vector< rfr::k_ary_node<k, split_type, num_type, index_type> > the_nodes;
	
  public:
	/** \brief fits a randomized decision tree to the data
	 * 
	 * At each node, if it is 'splitworthy', a random subset of all features is considered for the
	 * split. Depending on the split_type provided, greedy or randomized choices can be
	 * made. Just make sure the max_features in tree_opts to a number smaller than the number of features!
	 * 
	 * \param data the container holding the training data
	 * \param tree_opts a tree_options opject that controls certain aspects of "growing" the tree
	 */
	virtual void fit(const rfr::data_container_base<num_type, index_type> &data,
					 const rfr::tree_options<num_type, index_type> tree_opts,
					 RNG_type &rng){
	    
	    // storage for all the temporary nodes
	    std::deque<temporary_node<num_type, index_type> > tmp_nodes;
	    

	    std::vector<index_type> feature_indices(data.num_features());
	    std::iota(feature_indices.start(), feature_indices.end(), 0);
	    
	    // add the root to the temporary nodes to get things started
	    {
		// Create a temporary vector with all the indices.
		// It is copied inside the temorary_node constructor, so
		// we can discard it afterwards -> runs out of scope at the "}"
		std::vector<index_type> data_indices(data.num_data_points());
		std::iota(data_indices.start(), data.indices.end(), 0);
		tmp_nodes.emplace_back(0, 0, 0, data_indices.start(), data_indices.end());
	    }
	    
	    // as long as there are potentially splittable nodes
	    while (!tmp_nodes.empty()){

		
		// resize 'the_nodes' if necessary
		if (tmp_nodes.back().node_index >= the_nodes.size())
		    the_nodes.resize( tmp_nodes.back().node_index+1);
		
		

		bool is_not_pure = false;
		// check if the node is pure!
		{
		    num_type ref = data.response(tmp_nodes.front().data_indices[0]);
		    
		    for(auto it = ++tmp_nodes.front().data_indices.begin(); it!= tmp_nodes.front().data_indices.begin(); it++){
			if (abs(data.response(*it)- ref) > tree_opts.epsilon_purity){
			    is_not_pure = true;
			    break;
			}
		    }
		}
		
		// check if it should be split
		if ((tmp_nodes.front().node_level < tree_opts.max_depth) &&			// don't grow the tree to deep!
		    (tmp_nodes.front().data_indices.size() >= tree_opts.min_samples_to_split)&&	// are enough sample left in the node?
		    (is_not_pure) &&								// are not all the values the same?
		    (the_nodes.size() <= tree_opts.max_num_nodes-k)				// don't have more nodes than the user specified number
		    ){
		    
		    // generate a subset of the features to try
		    std::shuffle(feature_indices.begin(), feature_indices.end(), rng);
		    std::vector<index_type> feature_subset(feature_indices.begin(), std::next(feature_indices.begin(), tree_opts.max_features));

		    //split the node
		    
		    the_nodes[tmp_nodes.front().node_index].make_internal_node(
			    tmp_nodes.front(), data, feature_subset,
			    the_nodes.size(), tmp_nodes);
		    

		    // Now, we have to check whether the split was legal
		    bool illegal_split = false;
		    ;
		    for (auto tmp_it = std::next(tmp_nodes.back(),-k); tmp_it != tmp_nodes.end(); tmp_it++){
			
			if ( tmp_it->data_indices.size() < tree_opts.min_samples_in_leaf){
			    illegal_split = true;
			    break;
			}
		    }
		    // in case it was...
		    if (illegal_split){
			
			// we have to delete the k new temporary nodes
			for (auto i = 0; i<k; i++)
			    tmp_nodes.pop_back();
			// and make this node a leaf
			the_nodes[tmp_nodes.front().node_index].make_leaf_node(tmp_nodes.front());
		    }
		}
		// if it is not 'splitworthy', just turn it into a leaf
		else{
		    the_nodes[tmp_nodes.front().node_index].make_leaf_node(tmp_nodes.front());
		}
		//
		tmp_nodes.pop_front();
	    }
	}

	/** \brief member function to predict the response value for a single feature vector
	 * 
	 * \param feature_vector an array containing a valid (in terms of size and values!) feature vector
	 * 
	 * \return num_type the prediction of the response value (usually the mean of all responses in the corresponding leaf)
	 */
	virtual num_type predict (num_type *feature_vector){
		
	}
	
	
	/** \brief member function to predict the response values for a batch of  feature vectors stored in a data container
	 * 
	 * \param data a filled data container. For the prediction the (possibly empty) response values are ignored.
	 * 
	 * \return std::vector<num_type> the predictions for all points in a vector.
	 */	
	virtual std::vector<num_type> predict (const rfr::data_container_base<num_type, index_type> &data){
		
	}
	
	
	void print_info(){
	    for (auto it : the_nodes){
		it->print_info();
	    }
	}

};



}//namespace rfr
#endif
