#include <iostream>

template <typename... Types>
void print(const Types&... types){
    (std::cout << ... << types);
} 






int main(){
    int x;
    double y;
    long long z;
    print(x,y,z);
}