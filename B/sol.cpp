#include <iostream>
#include <sstream>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;


struct CustomTree;
struct CustomString;
using TreeUk = shared_ptr<CustomTree>;
TreeUk serialize(CustomString&);
TreeUk serialize_or(CustomString&);
TreeUk serialize_and(CustomString&);
TreeUk serialize_not(CustomString&);
TreeUk get_var_name(CustomString&);


typedef std::function<bool(const TreeUk&)> Function;
typedef std::vector<Function> Functions;


template<typename T>
void _hash_combine (size_t& seed, const T& val)
{
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template<typename... Types>
size_t hash_combine(const Types&... args)
{
    size_t seed = 1;
    (_hash_combine(seed, args), ...); // create hash value with seed over all args
    return seed;
} 


struct CustomTree{
    TreeUk left_tree;
    TreeUk right_tree;
    string content;
    size_t hash_code;
    
    CustomTree(const string c, const TreeUk l=NULL, const TreeUk r=NULL): 
        content(c), left_tree(l), right_tree(r){

        hash_code = hash<string>()(c);
        if (l != NULL) hash_code = hash_combine(hash_code, l->hash_code);
        if (r != NULL) hash_code = hash_combine(hash_code, r->hash_code);
    }
    
    static TreeUk build(const string c, const TreeUk l=NULL, const TreeUk r=NULL){
        return TreeUk(new CustomTree(c, l, r));
    }
    
    void print() const{
        if(type() == 1){
            cout << "(";
            left_tree -> print();
            cout << " " << content << " ";
            right_tree -> print();
            cout << ")";
        }else if(type() == 2){
            cout << content;
        }else{
            cout << content;
            left_tree -> print();
        }
    }
    
    bool is_implication() const {
        return content == "->" && left_tree != NULL && right_tree != NULL;
    }
    
    bool is_or() const {
        return content == "|" && left_tree != NULL && right_tree != NULL;
    }
    
    bool is_and() const {
        return content == "&" && left_tree != NULL && right_tree != NULL;
    }
    
    bool is_not() const {
        return content == "!" && left_tree != NULL && right_tree == NULL;
    }
    
    int8_t type() const{
        // 1 is full tree
        // 2 is only name
        // 3 is not tree
        
        if(left_tree != NULL && right_tree != NULL){
            return 1;
        }else if(left_tree == NULL && right_tree == NULL){
            return 2;
        }
        return 3;
    }
    
    bool equal_tree(const TreeUk& t1, signed long int l=9223372036854775807) const{
        if(l <= 0) return true;
        if(hash_code != t1 -> hash_code) return false;
        if(type() != t1 -> type()) return false;
        if(content != t1 -> content) return false;
        if(left_tree && !left_tree -> equal_tree(t1 -> left_tree, l-1)) return false;
        if(right_tree && !right_tree -> equal_tree(t1 -> right_tree, l-1)) return false;
        return true;
    }
};


struct myequal{
    bool operator()(const TreeUk& lhs,
                  const TreeUk& rhs) const
    {
        return lhs -> equal_tree(rhs);
    }
};

class myhash {
    public:
    size_t operator()(const TreeUk d1 ) const { 
        return d1 -> hash_code; 
    }
};

struct CustomString{
    string content;
    unsigned int i = 0;
    
    CustomString(string& c): content(c){
    }
    
    char get_next(){
        if(!is_end()){
            return content[i+1];
        }
        return '@';
    }
    
    void next(){
        if(!is_end()){
            i += 1;
        }
    }
    
    char current(){
        return content[i];
    }
    
    bool is_end(){
        return i == content.length();
    }
    
    bool current_is_part_of_var_name(){
        char c = current();
        return c != '(' && c != ')' && c != '!' && c != '&' && c != '|' && c != '!' && 
               c != ',' && c != '-' && c != '>' && c != ' ' && c != '\t' && c != '\r' && 
               c != '\n';
    }
    
    void skip_spaces(){
        while(current() == ' ' || current() == '\r' || current() == '\t' || current() == '\n') next();
    }
};


TreeUk serialize(CustomString& exp){
    exp.skip_spaces();
    TreeUk l = serialize_or(exp);
    exp.skip_spaces();
//    cout << exp.i << endl;
    if(exp.current() != '-' && exp.get_next() != '>') return l;
    exp.next(); exp.next();
//    cout << exp.i << endl;
    exp.skip_spaces();
    TreeUk r = serialize(exp);
    
    return CustomTree::build("->", l, r);
}


TreeUk serialize_or(CustomString& exp){
    exp.skip_spaces();
    TreeUk l = serialize_and(exp);
    
    exp.skip_spaces();
    while(exp.current() == '|'){
//        cout << exp.i << endl;
        exp.next();
        exp.skip_spaces();
        TreeUk r = serialize_and(exp);
        l = CustomTree::build("|", l, r);
    }
    
    return l;
}


TreeUk serialize_and(CustomString& exp){
    exp.skip_spaces();
    TreeUk l = serialize_not(exp);
    
    exp.skip_spaces();
    while(exp.current() == '&'){
//        cout << exp.i << " " << exp.current() << endl;
        exp.next();
        exp.skip_spaces();
        TreeUk r = serialize_not(exp);
//        cout << "can" << endl;
        l = CustomTree::build("&", l, r);
    }
    
    return l;
}


TreeUk serialize_not(CustomString& exp){
    exp.skip_spaces();
    if(exp.current() == '!'){
        exp.next();
        exp.skip_spaces();
        return CustomTree::build("!", serialize_not(exp));
    }
    
    if(exp.current() == '('){
        exp.next();
        exp.skip_spaces();
        TreeUk r = serialize(exp);
//        cout << exp.i << endl;
        exp.next();
        exp.skip_spaces();
        return r;
    }
    
    if(exp.is_end()) return NULL;
    string name = "";
    
    while(exp.current_is_part_of_var_name() && !exp.is_end()){
        name += exp.current();
        exp.next();
    }
    exp.skip_spaces();
    
//    cout << name << "." << endl;
    
    return CustomTree::build(name);
}


bool is_axiom1(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> right_tree -> is_implication() &&
            tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree)
            );
}

bool is_axiom2(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> right_tree -> is_implication() &&
            tree -> left_tree -> is_implication() &&
            tree -> right_tree -> right_tree -> is_implication() &&
            tree -> right_tree -> left_tree -> is_implication() &&
            tree -> right_tree -> left_tree -> right_tree -> is_implication() &&

            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> left_tree) &&
            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> left_tree -> left_tree) &&
            
            tree -> left_tree -> right_tree -> equal_tree(tree -> right_tree -> left_tree -> right_tree -> left_tree) &&
            
            tree -> right_tree -> right_tree -> right_tree -> equal_tree(tree -> right_tree -> left_tree -> right_tree -> right_tree)
            );
}

bool is_axiom3(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> right_tree -> is_implication() &&
            tree -> right_tree -> right_tree -> is_and() &&
            tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> left_tree) &&
            tree -> right_tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> right_tree)
            );
}

bool is_axiom4(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> left_tree -> is_and() &&
            tree -> right_tree -> equal_tree(tree -> left_tree -> left_tree)
            );
}

bool is_axiom5(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> left_tree -> is_and() &&
            tree -> right_tree -> equal_tree(tree -> left_tree -> right_tree)
            );
}

bool is_axiom6(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> right_tree -> is_or() &&
            tree -> left_tree -> equal_tree(tree -> right_tree -> left_tree)
            );
}

bool is_axiom7(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> right_tree -> is_or() &&
            tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree)
            );
}

bool is_axiom8(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> left_tree -> is_implication() &&
            tree -> right_tree -> is_implication() &&
            tree -> right_tree -> left_tree -> is_implication() &&
            tree -> right_tree -> right_tree -> is_implication () &&
            tree -> right_tree -> right_tree -> left_tree -> is_or() &&
            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> left_tree -> left_tree) &&
            tree -> left_tree -> right_tree -> equal_tree(tree -> right_tree -> left_tree -> right_tree) &&
            tree -> left_tree -> right_tree -> equal_tree(tree -> right_tree -> right_tree -> right_tree) &&
            tree -> right_tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> left_tree -> right_tree)
            );
}

bool is_axiom9(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> left_tree -> is_implication() &&
            tree -> right_tree -> is_implication() &&
            tree -> right_tree -> left_tree -> is_implication() &&
            tree -> right_tree -> left_tree -> right_tree -> is_not() &&
            tree -> right_tree -> right_tree -> is_not() &&
            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> left_tree -> left_tree) &&
            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> left_tree -> left_tree) &&
            tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree -> right_tree -> left_tree) &&
            tree -> left_tree -> right_tree -> equal_tree(tree -> right_tree -> left_tree -> right_tree -> left_tree)
            );
}

bool is_axiom10(const TreeUk& tree){
    return (tree -> is_implication() &&
            tree -> left_tree -> is_not() &&
            tree -> left_tree -> left_tree -> is_not() &&
            tree -> left_tree -> left_tree -> left_tree -> equal_tree(tree -> right_tree)
            );
}


Functions axioms_functions = {is_axiom1, is_axiom2, is_axiom3, is_axiom4, is_axiom5, is_axiom6, is_axiom7, is_axiom8, is_axiom9, is_axiom10};


int8_t which_axiom(const TreeUk& tree){
    for(int i = 0; i < axioms_functions.size(); i++){
        if(axioms_functions[i](tree)){
            return i + 1;
        }
    }
    return -1;
}


vector<string> split(string& s, const string& delimiter){
    vector<string> elements;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        elements.push_back(token);
        s.erase(0, pos + delimiter.length());
    }

    elements.push_back(s);

    return elements;
}



unordered_map<TreeUk, pair<int, int>, myhash, myequal> modus_ponens;
unordered_map<int, pair<int, int>> modus_ponens_ind;

unordered_map<TreeUk, int, myhash, myequal> hypothesis_map, expressions_map;
vector<pair<TreeUk, pair<string, int>>> answers;
vector<TreeUk> expressions, hypothesis;


pair<string, int> determine_case(TreeUk& exp_tree){
    if(hypothesis_map.count(exp_tree) != 0){
        return {"Hypothesis", hypothesis_map[exp_tree]};
    }

    int axiom_num = which_axiom(exp_tree);
    if(axiom_num != -1){
        return {"Ax. sch.", axiom_num};
    }

    return {"M.P.", -1};
}


bool compare_answers(pair<TreeUk, pair<string, int>>& a, pair<TreeUk, pair<string, int>>& b){
    return expressions_map[a.first] < expressions_map[b.first];
}


int solve_B(){
    string start_expression;

    getline(cin, start_expression);
    istringstream ss(start_expression);

    vector<string> splitted_A = split(start_expression, "|-");

    for(string hypothesis_str: split(splitted_A[0], ",")){
        if(hypothesis_str == "") break;
        CustomString hyp = CustomString(hypothesis_str);
        TreeUk hyp_tree = serialize(hyp);

        if(hypothesis_map.count(hyp_tree) == 0){
            hypothesis.push_back(hyp_tree);
            hypothesis_map[hyp_tree] = hypothesis_map.size() + 1;
        }
    }
    
    CustomString result = CustomString(splitted_A[1]);
    TreeUk result_tree = serialize(result);

//    cout << endl;
//    result_tree -> print();
//    cout << endl;
    
    TreeUk last_tree;
    bool fl = true;
    string exp;
    
    while(getline(cin, exp)){
        CustomString s_exp = CustomString(exp);
        TreeUk exp_tree = serialize(s_exp);
        last_tree = exp_tree;
        if(fl && expressions_map.count(exp_tree) == 0){
            expressions.push_back(exp_tree);
            expressions_map[exp_tree] = expressions_map.size();
        }
        if(last_tree -> equal_tree(result_tree)) fl = false;
    }  

    if(determine_case(last_tree).first == "Hypothesis") return -1;
    if(!last_tree -> equal_tree(result_tree)) return -1;

    for(int i = 0; i < expressions.size(); i++){
        TreeUk exp_tree = expressions[i]; 
        pair<string, int> exp_case = determine_case(exp_tree);

        if(exp_case.second == -1){
            if(modus_ponens.count(exp_tree) == 0){
                return -1;
            }

            int ind1 = modus_ponens[exp_tree].first;
            int ind2 = modus_ponens[exp_tree].second;

            modus_ponens_ind[i] = {ind1, ind2};
        }

        if(exp_tree -> is_implication()){
            if(expressions_map.count(exp_tree -> right_tree) != 0 &&
               expressions_map.count(exp_tree -> left_tree) != 0){
                int ind1 = expressions_map[exp_tree -> left_tree];
                int ind2 = expressions_map[exp_tree -> right_tree];

                if(ind1 == ind2) continue;

                modus_ponens[exp_tree -> right_tree] = {i, ind1}; 
            }
        }
    }

    vector<int> queue;
    vector<bool> visited_indexes;
    for(int i=0; i<expressions.size(); i++) visited_indexes.push_back(false);

    queue.push_back(expressions.size()-1);

    while(!queue.empty()){
        int ind = queue[0];
        queue.erase(queue.begin());

        if(visited_indexes[ind]) continue;
        visited_indexes[ind] = true;

        pair<string, int> exp_case = determine_case(expressions[ind]);

        if(exp_case.second != -1) continue;
        if(modus_ponens_ind.count(ind) == 0) return -1;

        int ind1 = modus_ponens_ind[ind].first;
        int ind2 = modus_ponens_ind[ind].second;

        queue.push_back(ind1);
        queue.push_back(ind2);
    }
    
    if(hypothesis_map.size() > 0){
        for(int i = 0; i < hypothesis.size() - 1; i++){
            hypothesis[i] -> print();
            cout << ", ";
        }
        hypothesis[hypothesis.size()-1] -> print();
        cout << " |- ";
    }else{
        cout << "|- ";
    }
    
    result_tree -> print();
    cout << endl;
    
    int curr_number = 1;
    unordered_map<int, int> curr_indexes;

    for(int i = 0; i < visited_indexes.size(); i++){
        if(visited_indexes[i] == false) continue;

        TreeUk res = expressions[i];
        pair<string, int> exp_case = determine_case(res);
        
        string method;

        if(exp_case.second == -1){
            int ind1 = curr_indexes[modus_ponens_ind[i].first];
            int ind2 = curr_indexes[modus_ponens_ind[i].second];

            method = "M.P. " + to_string(ind1) + ", " + to_string(ind2);
        }else{
            method = exp_case.first + " " + to_string(exp_case.second);
        }

        cout << "[" + to_string(curr_number) + ". " + method + "] ";
        res -> print();
        cout << endl;
        curr_indexes[i] = curr_number;
        curr_number++;
    }

    return 0;
}


int main(){
    if(solve_B() == -1){
        cout << "Proof is incorrect";
    }

    // string s;
    // cin >> s;
    // CustomString c = CustomString(s);
    // serialize(c) -> print();
    
//    vector<string> axioms = {"A->B->A", "(1->2)->(1->2->3)->(1->3)", "1->2->1&2", "1&2->1", "1&2->2", "1->1|2", "2->1|2", "(1->3)->(2->3)->(1|2->3)", "(1->2)->(1->!2)->!1", "!!1->1"};
//
//    for(string axiom: axioms){
//        CustomString b = CustomString(axiom);
//        TreeUk t = serialize(b);
//        int ax_num = which_axiom(t);
//        cout << ax_num << endl;
//    }
    
    return 0;
}
