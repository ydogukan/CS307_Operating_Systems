#include <iostream>
#include <list>
#include <iterator>

using namespace std;

struct Node {
    int ID;
    int size;
    int index;

    Node(int myID, int mySize, int myIndex)
    : ID(myID), size(mySize), index(myIndex) {}
};

ostream& operator << (ostream& os, const Node& myNode) {
    os << "[" << myNode.ID << "][" << myNode.size << "][" << myNode.index << "]";
    return os;
}

class HeapManager {
    private: 
    list<Node> heap;
    pthread_mutex_t mutex;

    public:
    int initHeap(int size) {
        heap.push_back(Node(-1, size, 0));
        pthread_mutex_init(&mutex, NULL);
        print();
        return 1;
    }

    int myMalloc(int ID, int size) {
        list<Node> :: iterator it;

        pthread_mutex_lock(&mutex);
        for (it = heap.begin(); it != heap.end(); it++) { // iterate over all the elements in the list
            if (it->ID == -1 && size < it->size) { // if the node is a free node with enough space
                // split the node
                // the first node represents the newly allocated space
                heap.insert(it, Node(ID, size, it->index)); 
                // the second node represents the remaining free space
                it->size -= size; it->index += size;

                cout << "Allocated for thread " << ID << endl;
                print();

                pthread_mutex_unlock(&mutex);
                return it->index - size; // return the index of the newly allocated node
            }
        }

        cout << "Can not allocate, requested size " << size << " for thread " << ID 
            << " is bigger than remaining size" << endl;
        print();

        pthread_mutex_unlock(&mutex);
        return -1;
    }

    int myFree(int ID, int index) {
        list<Node> :: iterator it;

        pthread_mutex_lock(&mutex);
        for (it = heap.begin(); it != heap.end(); it++) { // iterate over all the elements in the list
            if (it->ID == ID && it->index == index) { // if the node has the given ID and index
                it->ID = -1; // turn the node into a free node

                if (prev(it)->ID == -1) { // if the left neighbour is also free
                    it->size += prev(it)->size;
                    it->index = prev(it)->index;
                    heap.erase(prev(it));
                }

                if (next(it)->ID == -1) { // if the right neighbour is also free
                    it->size += next(it)->size;
                    heap.erase(next(it));
                }
                
                cout << "Freed for thread " << ID << endl;
                print();

                pthread_mutex_unlock(&mutex);
                return 1;
            }
        }
        cout << "Can not free, requested node with ID " << ID << " and index " << index << " for thread " << ID 
            << " does not exist" << endl;
        print();

        pthread_mutex_unlock(&mutex);
        return -1;
    }

    void print() {
        list<Node> :: iterator it;
        it = heap.begin();
        cout << *(it++);

        for (; it != heap.end(); it++) {
            cout << "---" << *it;
        }
        cout << endl;
    }
};