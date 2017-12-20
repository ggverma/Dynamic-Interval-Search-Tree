#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <conio.h>
#include<queue>
#include<list>
#include<iomanip>
#include<set>
using namespace std;

#define BLOCKSIZE 100

// Struct defined for the node to store of the enpoints for an interval (will be the node for the AVL tree)
struct m_tree_t {
	int				value;				/* The value of the end-point as the key of the node */
	bool			isLeft;				/* If the end-point is the left end-point. To hold the value of 'which' field */
	int				other;				/* The other end-point of the interval */

	set<int>		*listOfOtherPoints;	/* All the other points associated with this node.*/

	int				min;				/* Minimum value in the sub-tree */
	int				max;				/* Maximum value in the sub-tree */
	int				leftMin;			/* Minimum left end-point value */
	int				rightMax;			/* Maximum right endpoint value */
	int				measure;			/* Measure of the line segments represented by the sub-tree */



	struct m_tree_t	*left;				/* Left child */
	struct m_tree_t	*right;				/* Right Child */
	int				height;				/* height of the node - to be used for balancing the tree */
	bool			isHead;				/* flag for the root node of the text-editor, created by 'create_text' function,
										root node will not contain any text */
};

m_tree_t	*currentblock = NULL;					/* Block of new nodes. */
m_tree_t	*free_list = NULL;						/* Nodes return to this list after deletion */
int			nodes_taken = 0;						/* Total number of lines in text. nodes_taken = N */
int			nodes_returned = 0;						/* Total number of nodes in free list / total nodes deleted */
int			size_left;								/* Nodes left in currentblock */


													/*-------------------------------------------------------------------------
													Functions for maintaining free list and allocated block
													---------------------------------------------------------------------------
													*/

													/*
													Return a node from currentblock / free_list.
													First checks in free list.
													If no node is free, checks in currentblock
													If currentblock is Empty, currentblock is initialzed to an array of size
													BLOCKSIZE and the first node is returned.
													*/
m_tree_t *get_node() {
	m_tree_t *tmp;

	nodes_taken += 1;

	if (free_list != NULL)
	{
		tmp = free_list;
		free_list = free_list->right;
	}
	else
	{
		if (currentblock == NULL || size_left == 0)
		{
			currentblock = (m_tree_t *)malloc(BLOCKSIZE * sizeof(m_tree_t));
			size_left = BLOCKSIZE;
		}

		tmp = currentblock++;
		size_left -= 1;
	}

	// Fresh node
	// Reset structural properties
	tmp->left = NULL;
	tmp->right = NULL;
	tmp->height = 0;
	tmp->isHead = false;

	// Reset other properties
	tmp->value = 0;
	tmp->isLeft = 0;
	tmp->other = 0;
	tmp->min = 0;
	tmp->max = 0;
	tmp->leftMin = 0;
	tmp->rightMax = 0;
	tmp->measure = 0;
	tmp->listOfOtherPoints = new set<int>;

	return(tmp);
}

/*
Adds the node to the free list
*/
void add_to_free_list(m_tree_t *node) {
	node->right = free_list;
	node->listOfOtherPoints->clear();
	free_list = node;
	nodes_returned += 1;
}

/*
Function to return the minimum of three numbers
*/
int getMin(int num1, int num2, int num3) {
	return std::min(num1, min(num2, num3));
}

/*
Function to return the minimum of three numbers
*/
int getMax(int num1, int num2, int num3) {
	return max(num1, max(num2, num3));
}

void computeMeasure(m_tree_t *node) {

	if (node != NULL) {

		if (node->isLeft) {
			// Case: If current node is the left node
			if (node->left != NULL && node->right != NULL) {
				// Case 0
				if (node->left->rightMax >= node->right->max) {
					node->measure = node->left->measure + node->right->max - node->left->max;
				}
				else if (node->right->leftMin <= node->left->min) {
					if (node->other <= node->right->max) {
						node->measure = node->right->measure + node->right->min - node->left->min;
					}
					else {
						node->measure = node->right->max - node->left->min;
					}
				}
				// Case 1
				else if (((node->right->leftMin == node->right->min) || (node->right->leftMin == node->value)) && (node->left->rightMax == node->left->max)) {
					if (node->other <= node->right->max) {
						node->measure = node->left->measure + node->right->measure + node->right->min - node->value;
					}
					else {
						node->measure = node->left->measure + node->right->max - node->value;
					}
				}
				// Case 2
				else if (node->left->rightMax != node->left->max) {
					if (node->other <= node->right->max) {
						// Other is contained.
						if (node->left->rightMax >= node->right->max) {
							node->measure = node->left->measure + node->right->max - node->left->max;
						}
						else {
							node->measure = node->left->measure + node->right->measure + node->right->min - node->left->max;
						}
					}
					else {
						// Other is NOT contained.
						node->measure = node->left->measure + node->right->max - node->left->max;
					}
				}
				// Case 3
				//else if (node->right->leftMin != node->right->min && node->right->leftMin != node->value) {
				else {
					if (node->other <= node->right->max) {
						// Other is contained.
						//node->measure = node->left->measure + node->right->measure + node->right->min - node->value;
						if (node->right->leftMin <= node->left->min) {
							node->measure = node->right->measure + node->right->min - node->left->min;
						}
						else {
							node->measure = node->left->measure + node->right->measure + node->right->min - node->left->max;
						}
					}
					else {
						// Other is NOT contained.
						//node->measure = node->right->measure + node->right->min - node->left->min;
						if (node->right->leftMin <= node->left->min) {
							node->measure = node->right->max - node->left->min;
						}
						else {
							node->measure = node->left->measure + node->right->max - node->left->max;
						}
					}
				}
			}
			else {
				// TODO: check the conditions in which left/right child is null
				if (node->left == NULL && node->right == NULL) {
					node->measure = 0;
				}
				else {

					if (node->left != NULL) {
						// Right exists
						if (node->left->rightMax == node->left->max) {
							node->measure = node->left->measure;
						}
						else {
							node->measure = node->left->measure + node->value - node->left->max;
						}
					}

					if (node->right != NULL) {
						// Left is null.
						if (node->other <= node->right->max) {
							node->measure = node->right->measure + node->right->min - node->value;
						}
						else {
							node->measure = node->right->max - node->value;
						}
					}
				}
			}
		}
		else {
			// Case: If current node is the right node
			if (node->left != NULL && node->right != NULL) {

				// Case 0
				if (node->right->leftMin <= node->left->min) {
					node->measure = node->right->measure + node->right->min - node->left->min;
				}
				else if (node->left->rightMax >= node->right->max) {
					if (node->other <= node->right->max) {
						node->measure = node->left->measure + node->right->max - node->left->max;
					}
					else {
						node->measure = node->right->max - node->left->min;
					}
				}
				// Case 1
				else if (((node->left->rightMax == node->left->max) || (node->left->rightMax == node->value)) && (node->right->leftMin == node->right->min)) {
					if (node->other >= node->left->min) {
						node->measure = node->left->measure + node->right->measure + node->value - node->left->max;
					}
					else {
						node->measure = node->right->measure + node->value - node->left->min;
					}
				}
				// Case 2
				else if (node->right->leftMin != node->right->min) {
					if (node->other >= node->left->min) {
						// Other is contained.
						if (node->right->leftMin <= node->left->min) {
							node->measure = node->right->measure + node->right->min - node->left->min;
						}
						else {
							node->measure = node->left->measure + node->right->measure + node->right->min - node->left->max;
						}
					}
					else {
						// Other is NOT contained.
						node->measure = node->right->measure + node->right->min - node->left->min;
					}
				}
				// Case 3
				//else if (node->left->rightMax != node->left->max && node->left->rightMax != node->value) {
				else {
					if (node->other >= node->left->min) {
						// Other is contained.
						//node->measure = node->left->measure + node->right->measure + node->value - node->left->max;
						if (node->right->leftMin <= node->left->min) {
							node->measure = node->left->measure + node->right->max - node->left->max;
						}
						else {
							node->measure = node->left->measure + node->right->measure + node->right->min - node->left->max;
						}
					}
					else {
						// Other is NOT contained.
						//node->measure = node->left->measure + node->right->max - node->left->max;
						if (node->right->leftMin <= node->left->min) {
							node->measure = node->right->max - node->left->min;
						}
						else {
							node->measure = node->right->measure + node->right->min - node->left->min;
						}
					}
				}
			}
			else {
				// TODO: check the conditions in which left/right child is null
				if (node->left == NULL && node->right == NULL) {
					node->measure = 0;
				}
				else {
					if (node->right != NULL) {
						// Right exists.
						if (node->right->leftMin == node->right->min) {
							node->measure = node->right->measure;
						}
						else {
							node->measure = node->right->measure + node->right->min - node->value;
						}
					}
					else {
						// Left exists
						if (node->other >= node->left->min) {
							node->measure = node->left->measure + node->value - node->left->max;
						}
						else {
							node->measure = node->value - node->left->min;
						}
					}
				}
			}
		}
	}
}

/*
Method to resconstruct the node properties
*/
void nodeReconstruction(m_tree_t *node) {

	// Setting up min(node)
	if (node->left != NULL)
		node->min = node->left->min;
	else
		node->min = node->value;

	// Setting up max(node)
	if (node->right != NULL)
		node->max = node->right->max;
	else
		node->max = node->value;

	// Setting up leftMin(node)
	int nodeVal;
	if (node->isLeft == true)
		nodeVal = node->value;
	else
		nodeVal = node->other;

	if (node->left != NULL && node->right != NULL) {
		node->leftMin = getMin(nodeVal, node->left->leftMin, node->right->leftMin);
	}
	else {
		if (node->left != NULL)
			node->leftMin = min(nodeVal, node->left->leftMin);
		else if (node->right != NULL)
			node->leftMin = min(nodeVal, node->right->leftMin);
		else {
			node->leftMin = nodeVal;
		}
	}

	// Setting up rightMax(node)
	if (node->isLeft == false)
		nodeVal = node->value;
	else
		nodeVal = node->other;

	if (node->left != NULL && node->right != NULL) {
		node->rightMax = getMax(nodeVal, node->left->rightMax, node->right->rightMax);
	}
	else {
		if (node->left != NULL)
			node->rightMax = max(nodeVal, node->left->rightMax);
		else if (node->right != NULL)
			node->rightMax = max(nodeVal, node->right->rightMax);
		else {
			node->rightMax = nodeVal;
		}
	}

	// Compute the measure of the node
	computeMeasure(node);
}

/*-------------------------------------------------------------------------
Functions related to balancing the AVL tree
---------------------------------------------------------------------------
*/


/*
Function to get the height of the left child
*/
int getLeftHeight(m_tree_t * node) {
	return (node->left != NULL) ? node->left->height + 1 : 0;
}

/*
Function to get the height of the right child
*/
int getRightHeight(m_tree_t *node) {
	return (node->right != NULL) ? node->right->height + 1 : 0;
}

/*
Function to check if the sub-tree at a given node is balanced
*/
bool isNodeBalanced(m_tree_t *node) {
	bool balance = true;

	if (node != NULL) {
		int leftHeight = getLeftHeight(node);
		int rightHeight = getRightHeight(node);

		if (abs(leftHeight - rightHeight) > 1)
			balance = false;
	}

	return balance;
}

/*
Function to increment the node height of the given node
*/
void update_node_height(m_tree_t *node) {
	int leftHeight = getLeftHeight(node);
	int rightHeight = getRightHeight(node);

	node->height = (leftHeight > rightHeight) ? leftHeight : rightHeight;
}


/*-------------------------------------------------------------------------
Functions for rotation and rebalancing
---------------------------------------------------------------------------
*/

/*
Function to balance the tree with zig-zig pattern
*/
m_tree_t * rotate_zig_zig(m_tree_t  *unbalanced_node, bool isleft) {
	m_tree_t *tmp = NULL;

	if (isleft) {
		// Do a right rotation
		tmp = unbalanced_node->left;

		unbalanced_node->left = unbalanced_node->left->right;
		tmp->right = unbalanced_node;
	}
	else {
		// Do a left rotation
		tmp = unbalanced_node->right;

		unbalanced_node->right = unbalanced_node->right->left;
		tmp->left = unbalanced_node;
	}

	// Adjust the measures and other properties
	tmp->min = unbalanced_node->min;
	tmp->max = unbalanced_node->max;
	tmp->leftMin = unbalanced_node->leftMin;
	tmp->rightMax = unbalanced_node->rightMax;
	tmp->measure = unbalanced_node->measure;


	// Adjust the properties of the unbalanced node
	nodeReconstruction(unbalanced_node);

	// update the node height for the AVL tree
	update_node_height(unbalanced_node);
	update_node_height(tmp);

	return tmp;
}

/*
Function to balance the tree with zig-zag pattern
*/
m_tree_t * rotate_zig_zag(m_tree_t *unbalanced_node, bool isleft) {
	m_tree_t *tmp = NULL;

	if (isleft) {
		tmp = unbalanced_node->left->right;

		unbalanced_node->left->right = tmp->left;
		tmp->left = unbalanced_node->left;
		unbalanced_node->left = tmp->right;
		tmp->right = unbalanced_node;
	}
	else
	{
		tmp = unbalanced_node->right->left;

		unbalanced_node->right->left = tmp->right;
		tmp->right = unbalanced_node->right;
		unbalanced_node->right = tmp->left;
		tmp->left = unbalanced_node;
	}

	// Adjust the measures and other properties
	tmp->min = unbalanced_node->min;
	tmp->max = unbalanced_node->max;
	tmp->leftMin = unbalanced_node->leftMin;
	tmp->rightMax = unbalanced_node->rightMax;
	tmp->measure = unbalanced_node->measure;

	// Adjust the properties of the child nodes
	nodeReconstruction(tmp->left);
	nodeReconstruction(tmp->right);

	// Adjust the heights of the nodes for balancing
	update_node_height(tmp);
	update_node_height(tmp->left);
	update_node_height(tmp->right);

	return tmp;
}

/*
Function to check if the child sub-trees at a given node are balanced
Balances the child nodes in case they are not
*/
m_tree_t * rebalance(m_tree_t *node) {

	// Checking the balance of the left child
	if (!isNodeBalanced(node)) {

		if (getLeftHeight(node) > getRightHeight(node))
		{
			if (getLeftHeight(node->left) > getRightHeight(node->left)) {
				// rotate zig-zig left to rebalance the left-child of the node
				return rotate_zig_zig(node, true);
			}
			else {
				// rotate zig-zig left to rebalance the left-child of the node
				return rotate_zig_zag(node, true);
			}
		}
		else {
			if (getLeftHeight(node->right) > getRightHeight(node->right)) {
				// rotate zig-zag left to rebalance the left-child of the node
				return rotate_zig_zag(node, false);
			}
			else {
				// rotate zig-zig left to rebalance the left-child of the node
				return rotate_zig_zig(node, false);
			}
		}
	}
	return node;
}

void transferContentsOfN1ToN2(m_tree_t *n1, m_tree_t *n2) {
	n2->value = n1->value;
	n2->other = n1->other;
	n2->isLeft = n1->isLeft;
	n2->min = n1->min;
	n2->max = n1->max;
	n2->leftMin = n1->leftMin;
	n2->rightMax = n1->rightMax;
	n2->listOfOtherPoints = n1->listOfOtherPoints;
}

/*-------------------------------------------------------------------------
Inserting a new node
---------------------------------------------------------------------------
*/


m_tree_t * insert(m_tree_t *node, int leftPoint, int rightPoint, bool isSearchingBoth, bool isSearchingLeft) {
	if (node == NULL) {
		node = get_node();
		if (isSearchingLeft) {
			node->value = leftPoint;
			node->other = rightPoint;
			node->isLeft = true;
		}
		else {
			node->value = rightPoint;
			node->other = leftPoint;
			node->isLeft = false;
		}
	}
	else if (node->left != NULL && node->isHead) {
		m_tree_t *balancedNodeOnLeft = insert(node->left, leftPoint, rightPoint, true, false);
		node->left = balancedNodeOnLeft;
	}
	else if (node->left == NULL && node->isHead) {
		// CASE B.
		node->left = get_node();
		node = node->left;
		node->value = leftPoint;
		node->other = rightPoint;
		node->isLeft = true;

		if (leftPoint != rightPoint) {
			node->height = 1;
			node->right = get_node();
			node->right->value = rightPoint;
			node->right->other = leftPoint;
			node->right->isLeft = false;
			// Adjust the measure related properties of the node
			nodeReconstruction(node->right);
		}
		// Adjust the measure related properties of the node
		nodeReconstruction(node);
		return node;
	}
	else if (isSearchingBoth) {
		// Search.
		if (node->value >= leftPoint && node->value <= rightPoint) {
			// (LeftPoint matches or left point is in left subtree) and (rightPoint matches or it is in right subtree)
			// CASE A.
			if (node->value != leftPoint) {
				m_tree_t *balancedNodeOnLeft = insert(node->left, leftPoint, rightPoint, false, true);
				node->left = balancedNodeOnLeft;
			}
			else {
				if (node->isLeft) {
					node->listOfOtherPoints->insert(rightPoint);
					node->other = node->other > rightPoint ? node->other : rightPoint;
				}
			}

			if (node->value != rightPoint) {
				m_tree_t *balancedNodeOnRight = insert(node->right, leftPoint, rightPoint, false, false);
				node->right = balancedNodeOnRight;
			}
			else {
				if (!node->isLeft) {
					node->listOfOtherPoints->insert(leftPoint);
					node->other = node->other < leftPoint ? node->other : leftPoint;
				}
			}
		}
		else if (node->value < rightPoint && node->value < leftPoint) {
			// Both to the right.
			// CASE B.
			if (node->right != NULL) {
				m_tree_t *balancedNodeOnRight = insert(node->right, leftPoint, rightPoint, true, false);
				node->right = balancedNodeOnRight;
			}
			else {
				node->right = insert(node->right, leftPoint, rightPoint, false, true);

				node->right->right = insert(node->right->right, leftPoint, rightPoint, false, false);
				// Adjust the height of the current node
				update_node_height(node->right->right);

				// Adjust the measure related properties of the node
				nodeReconstruction(node->right->right);
				// Adjust the height of the current node
				update_node_height(node->right);

				// Adjust the measure related properties of the node
				nodeReconstruction(node->right);
			}
		}
		else {
			// Both to the left.
			// CASE B.
			if (node->left != NULL) {
				m_tree_t *balancedNodeOnLeft = insert(node->left, leftPoint, rightPoint, true, false);
				node->left = balancedNodeOnLeft;
			}
			else {
				node->left = insert(node->left, leftPoint, rightPoint, false, false);

				node->left->left = insert(node->left->left, leftPoint, rightPoint, false, true);
				// Adjust the height of the current node
				update_node_height(node->left->left);

				// Adjust the measure related properties of the node
				nodeReconstruction(node->left->left);
				// Adjust the height of the current node
				update_node_height(node->left);

				// Adjust the measure related properties of the node
				nodeReconstruction(node->left);
			}
		}
	}
	else if (isSearchingLeft) {
		// Only for left.
		if (node->value < leftPoint) {
			// Find and insert on right.
			m_tree_t *balancedNodeOnRight = insert(node->right, leftPoint, rightPoint, false, true);
			node->right = balancedNodeOnRight;
		}
		else if (node->value > leftPoint) {
			// Find and insert on left.
			m_tree_t *balancedNodeOnLeft = insert(node->left, leftPoint, rightPoint, false, true);
			node->left = balancedNodeOnLeft;
		}
		else {
			// Same node!
			node->listOfOtherPoints->insert(rightPoint);
			node->other = node->other > rightPoint ? node->other : rightPoint;
		}
	}
	else {
		// Only for right.
		if (node->value < rightPoint) {
			// Find and insert on right.
			m_tree_t *balancedNodeOnRight = insert(node->right, leftPoint, rightPoint, false, false);
			node->right = balancedNodeOnRight;
		}
		else if (node->value > rightPoint) {
			// Find and insert on left.
			m_tree_t *balancedNodeOnLeft = insert(node->left, leftPoint, rightPoint, false, false);
			node->left = balancedNodeOnLeft;
		}
		else {
			// Same node!
			node->listOfOtherPoints->insert(leftPoint);
			node->other = node->other < leftPoint ? node->other : leftPoint;
		}
	}

	// Adjust the height of the current node
	update_node_height(node);

	// Adjust the measure related properties of the node
	nodeReconstruction(node);

	// rebalance the child-subtrees if required
	if (node->isHead)
		return node;
	else
		return rebalance(node);
}

m_tree_t * delete_node(m_tree_t *node, int leftPoint, int rightPoint, bool isSearchingBoth, bool isSearchingLeft) {
	if (node == NULL) {
		return NULL;
	}
	else if (node->left != NULL && node->isHead) {
		m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, true, false);
		node->left = balancedNodeOnLeft;
	}
	else if (node->left == NULL && node->isHead) {
		return NULL;
	}
	else if (isSearchingBoth) {
		// Search.
		if (node->value >= leftPoint && node->value <= rightPoint) {
			// Forked here.
			// (LeftPoint matches or left point is in left subtree) and (rightPoint matches or it is in right subtree)
			// CASE A.
			if (node->value == leftPoint && leftPoint == rightPoint) {
				// Delete this node.
				if (node->listOfOtherPoints->empty()) {
					if (node->left == NULL && node->right == NULL) {
						add_to_free_list(node);
						return NULL;
					}
					else if (node->left == NULL) {
						m_tree_t *nextNode = node->right;
						node->right = NULL;
						add_to_free_list(node);
						return nextNode;
					}
					else if (node->right == NULL) {
						m_tree_t *nextNode = node->left;
						node->left = NULL;
						add_to_free_list(node);
						return nextNode;
					}
					else {
						m_tree_t *minimaNode = node->right;
						while (minimaNode->left != NULL) minimaNode = minimaNode->left;
						transferContentsOfN1ToN2(minimaNode, node);
						minimaNode->listOfOtherPoints->clear();
						m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
						node->right = balancedNodeOnRight;
					}
				}
			}
			else if (node->value == leftPoint) {
				// First delete right point and then delete this node.
				m_tree_t *balancedNodeOnRight = delete_node(node->right, leftPoint, rightPoint, false, false);
				node->right = balancedNodeOnRight;

				node->listOfOtherPoints->erase(rightPoint);
				if (node->listOfOtherPoints->empty()) {
					if (node->left == NULL) {
						// Does not matter if node->right is NULL.
						m_tree_t *nextNode = node->right;
						node->right = NULL;
						add_to_free_list(node);
						return nextNode;
					}
					else {
						if (node->right == NULL) {
							// Only left child exists.
							m_tree_t *nextNode = node->left;
							node->left = NULL;
							add_to_free_list(node);
							return nextNode;
						}
						else {
							// Both child exist.
							m_tree_t *minimaNode = node->right;
							while (minimaNode->left != NULL) minimaNode = minimaNode->left;
							transferContentsOfN1ToN2(minimaNode, node);
							minimaNode->listOfOtherPoints->clear();
							m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
							node->right = balancedNodeOnRight;
						}
					}
				}
				else {
					if (node->isLeft) {
						if (*(node->listOfOtherPoints->rbegin()) >= node->value) {
							node->other = *(node->listOfOtherPoints->rbegin());
						}
						else {
							node->isLeft = false;
							node->other = *(node->listOfOtherPoints->begin());
						}
					}
					else {
						if (*(node->listOfOtherPoints->begin()) <= node->value) {
							node->other = *(node->listOfOtherPoints->begin());
						}
						else {
							node->isLeft = true;
							node->other = *(node->listOfOtherPoints->rbegin());
						}
					}
					return node;
				}
			}
			else if (node->value == rightPoint) {
				// First delete left point and then delete this node.
				m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, false, true);
				node->left = balancedNodeOnLeft;

				node->listOfOtherPoints->erase(leftPoint);
				if (node->listOfOtherPoints->empty()) {
					if (node->left == NULL) {
						// Does not matter if node->right is NULL.
						m_tree_t *nextNode = node->right;
						node->right = NULL;
						add_to_free_list(node);
						return nextNode;
					}
					else {
						if (node->right == NULL) {
							// Only left child exists.
							m_tree_t *nextNode = node->left;
							node->left = NULL;
							add_to_free_list(node);
							return nextNode;
						}
						else {
							// Both child exist.
							m_tree_t *minimaNode = node->right;
							while (minimaNode->left != NULL) minimaNode = minimaNode->left;
							transferContentsOfN1ToN2(minimaNode, node);
							minimaNode->listOfOtherPoints->clear();
							m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
							node->right = balancedNodeOnRight;
						}
					}
				}
				else {
					if (node->isLeft) {
						if (*(node->listOfOtherPoints->rbegin()) >= node->value) {
							node->other = *(node->listOfOtherPoints->rbegin());
						}
						else {
							node->isLeft = false;
							node->other = *(node->listOfOtherPoints->begin());
						}
					}
					else {
						if (*(node->listOfOtherPoints->begin()) <= node->value) {
							node->other = *(node->listOfOtherPoints->begin());
						}
						else {
							node->isLeft = true;
							node->other = *(node->listOfOtherPoints->rbegin());
						}
					}
					return node;
				}
				/*if (node->right == NULL) {
				m_tree_t *nextNode = node->left;
				node->left = NULL;
				add_to_free_list(node);
				return nextNode;
				}
				else {
				m_tree_t *minimaNode = node->right;
				while (minimaNode->left != NULL) minimaNode = minimaNode->left;
				transferContentsOfN1ToN2(minimaNode, node);
				m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
				node->right = balancedNodeOnRight;
				}*/
			}
			else {
				// Delete them. Order does not matter.
				m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, false, true);
				node->left = balancedNodeOnLeft;

				m_tree_t *balancedNodeOnRight = delete_node(node->right, leftPoint, rightPoint, false, false);
				node->right = balancedNodeOnRight;
			}
		}
		else if (node->value < rightPoint && node->value < leftPoint) {
			// Both to the right.
			// CASE B.
			m_tree_t *balancedNodeOnRight = delete_node(node->right, leftPoint, rightPoint, true, false);
			node->right = balancedNodeOnRight;
		}
		else {
			// Both to the left.
			// CASE B.
			m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, true, false);
			node->left = balancedNodeOnLeft;
		}
	}
	else if (isSearchingLeft) {
		// Only for left.
		if (node->value < leftPoint) {
			// Find and delete on right.
			m_tree_t *balancedNodeOnRight = delete_node(node->right, leftPoint, rightPoint, false, true);
			node->right = balancedNodeOnRight;
		}
		else if (node->value > leftPoint) {
			// Find and insert on left.
			m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, false, true);
			node->left = balancedNodeOnLeft;
		}
		else {
			// This is the node that contains left point.
			node->listOfOtherPoints->erase(rightPoint);
			if (node->listOfOtherPoints->empty()) {
				if (node->left == NULL && node->right == NULL) {
					add_to_free_list(node);
					return NULL;
				}
				else if (node->left == NULL) {
					m_tree_t *nextNode = node->right;
					node->right = NULL;
					add_to_free_list(node);
					return nextNode;
				}
				else if (node->right == NULL) {
					m_tree_t *nextNode = node->left;
					node->left = NULL;
					add_to_free_list(node);
					return nextNode;
				}
				else {
					m_tree_t *minimaNode = node->right;
					while (minimaNode->left != NULL) minimaNode = minimaNode->left;
					transferContentsOfN1ToN2(minimaNode, node);
					minimaNode->listOfOtherPoints->clear();
					m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
					node->right = balancedNodeOnRight;
				}
			}
			else {
				if (node->isLeft) {
					if (*(node->listOfOtherPoints->rbegin()) >= node->value) {
						node->other = *(node->listOfOtherPoints->rbegin());
					}
					else {
						node->isLeft = false;
						node->other = *(node->listOfOtherPoints->begin());
					}
				}
				else {
					if (*(node->listOfOtherPoints->begin()) <= node->value) {
						node->other = *(node->listOfOtherPoints->begin());
					}
					else {
						node->isLeft = true;
						node->other = *(node->listOfOtherPoints->rbegin());
					}
				}
				return node;
			}
		}
	}
	else {
		// Only for right.
		if (node->value < rightPoint) {
			// Find and insert on right.
			m_tree_t *balancedNodeOnRight = delete_node(node->right, leftPoint, rightPoint, false, false);
			node->right = balancedNodeOnRight;
		}
		else if (node->value > rightPoint) {
			// Find and insert on left.
			m_tree_t *balancedNodeOnLeft = delete_node(node->left, leftPoint, rightPoint, false, false);
			node->left = balancedNodeOnLeft;
		}
		else {
			// Same node!
			// This is the node that contains right point.
			node->listOfOtherPoints->erase(leftPoint);
			if (node->listOfOtherPoints->empty()) {
				if (node->left == NULL && node->right == NULL) {
					add_to_free_list(node);
					return NULL;
				}
				else if (node->left == NULL) {
					m_tree_t *nextNode = node->right;
					node->right = NULL;
					add_to_free_list(node);
					return nextNode;
				}
				else if (node->right == NULL) {
					m_tree_t *nextNode = node->left;
					node->left = NULL;
					add_to_free_list(node);
					return nextNode;
				}
				else {
					m_tree_t *minimaNode = node->right;
					while (minimaNode->left != NULL) minimaNode = minimaNode->left;
					transferContentsOfN1ToN2(minimaNode, node);
					minimaNode->listOfOtherPoints->clear();
					m_tree_t *balancedNodeOnRight = delete_node(node->right, minimaNode->value, rightPoint, false, true);
					node->right = balancedNodeOnRight;
				}
			}
			else {
				if (node->isLeft) {
					if (*(node->listOfOtherPoints->rbegin()) >= node->value) {
						node->other = *(node->listOfOtherPoints->rbegin());
					}
					else {
						node->isLeft = false;
						node->other = *(node->listOfOtherPoints->begin());
					}
				}
				else {
					if (*(node->listOfOtherPoints->begin()) <= node->value) {
						node->other = *(node->listOfOtherPoints->begin());
					}
					else {
						node->isLeft = true;
						node->other = *(node->listOfOtherPoints->rbegin());
					}
				}
				return node;
			}
		}
	}

	// Adjust the height of the current node
	update_node_height(node);

	// Adjust the measure related properties of the node
	nodeReconstruction(node);

	// rebalance the child-subtrees if required
	if (node->isHead)
		return node;
	else
		return rebalance(node);
}

/*-------------------------------------------------------------------------
Function to create a new tree
---------------------------------------------------------------------------
*/
m_tree_t * create_m_tree() {
	m_tree_t *tmp_node;
	tmp_node = get_node();

	// Setting up the attributes for the new node
	tmp_node->height = 0;
	tmp_node->isHead = true;

	return(tmp_node);
}

void preorder_traversal(m_tree_t * node);

/*-------------------------------------------------------------------------
Function to insert an interval
---------------------------------------------------------------------------
*/
void insert_interval(m_tree_t *tree, int a, int b) {
	/*cout << "Inserting interval (" << a << ", " << b << ")" << endl;
	if (a > b) {
	int c = a;
	a = b;
	b = c;
	}
	insert(tree, a, b, true, false);
	cout << "Tree pre_order: ";
	preorder_traversal(tree);
	cout << endl;*/
	if (a < b) {
		insert(tree, a, b, true, false);
		//cout << "Tree pre_order: ";
		//preorder_traversal(tree);
		//cout << endl;
	}
	else {
		cout << "Invalid pair" << endl;
	}
}


/*-------------------------------------------------------------------------
Function to delete an interval
---------------------------------------------------------------------------
*/
void delete_interval(m_tree_t * tree, int a, int b) {
	cout << "\nDeleting interval (" << a << ", " << b << ")" << endl;
	if (a > b) {
		int c = a;
		a = b;
		b = c;
	}
	delete_node(tree, a, b, true, false);
	//cout << "Tree pre_order: ";
	//preorder_traversal(tree);
	//cout << endl;
}

/*-------------------------------------------------------------------------
Function to get the query length
---------------------------------------------------------------------------
*/
int query_length(m_tree_t * tree) {
	if (tree != NULL && tree->isHead == false)
		return tree->measure;
	else if (tree != NULL && tree->isHead == true && tree->left != NULL)
		return tree->left->measure;
	else
		return 0;
}

/*-------------------------------------------------------------------------
Function to destroy a measure tree and release memory
---------------------------------------------------------------------------
*/
void destroy_m_tree(m_tree_t *tree) {
	if (tree == NULL)
		return;

	destroy_m_tree(tree->left);

	if (!tree->isHead) {
		add_to_free_list(tree);
	}


	destroy_m_tree(tree->right);
	if (tree->isHead) {
		tree->left = NULL;
	}
}


/*-------------------------------------------------------------------------
Functions to test
---------------------------------------------------------------------------
*/

void preorder_traversal(m_tree_t *node) {

	if (node == NULL)
		return;

	preorder_traversal(node->left);

	if (!node->isHead)
		cout << node->value << " ";

	preorder_traversal(node->right);

}

int pretty_print_tree_inorder(m_tree_t *node) {
	if (node == NULL) {
		return 0;
	}

	queue<m_tree_t *> q;

	int depth = node->left->height;

	q.push(node->left);

	int level = 0;

	int spaces = 0;
	for (; depth > 0; --depth) {
		spaces += pow(2, depth);
	}

	spaces /= 2;
	int inbetweenspaces = 1;

	while (inbetweenspaces > 0) {

		int k = q.size();

		for (int i = 0; i < spaces; ++i) {
			std::cout << "     ";
		}

		while (k > 0) {
			m_tree_t *n = q.front();

			q.pop();

			if (n != NULL) {
				std::cout << setw(5) << n->value << "(" << n->measure << ")";
			}
			else {
				std::cout << setw(5) << "-";
			}

			for (int i = 0; i < inbetweenspaces; ++i) {
				std::cout << "     ";
			}

			// Add the childs to the queue
			if (n != NULL) {
				if (n->left != NULL)
					q.push(n->left);
				else
					q.push(NULL);

				if (n->right != NULL)
					q.push(n->right);
				else
					q.push(NULL);
			}
			else {
				q.push(NULL);
				q.push(NULL);
			}

			k--;
		}
		cout << endl;

		inbetweenspaces = spaces;
		spaces /= 2;
	}
}


int main() {

	m_tree_t *root = create_m_tree();

	/*insert_interval(root, 1, 2);
	insert_interval(root, 3, 4);
	insert_interval(root, 6, 8);
	insert_interval(root, -10, 55);*/

	/*insert_interval(root, 1, 0);

	insert_interval(root, 1, 2);

	insert_interval(root, 2, 4);

	insert_interval(root, 6, 10);

	insert_interval(root, 7, 8);

	insert_interval(root, 7, 11);

	insert_interval(root, -1, 1);

	insert_interval(root, -5, -3);

	insert_interval(root, -6, -4);

	insert_interval(root, -7, 11);*/

	//pretty_print_tree_inorder(root);

	//delete_interval(root, -7, 11);

	//cout << endl << endl;

	//pretty_print_tree_inorder(root);

	for (int i = 0; i < 50; i++) {
		insert_interval(root, 2 * i, 2 * i + 1);
	}

	insert_interval(root, 0, 100);
	for (int i = 1; i < 50; i++) {
		insert_interval(root, 199 - (3 * i), 200); /*[52,200] is longest*/
	}
	std::cout << "\nQuery Length = " << query_length(root) << endl;

	for (int i = 2; i < 30; i++) {
		delete_interval(root, 2 * i, 2 * i + 1);
	}



	delete_interval(root, 60, 61);

	std::cout << "\nQuery Length = " << query_length(root) << endl;

	//pretty_print_tree_inorder(root);

	_getch();

	return 1;
}