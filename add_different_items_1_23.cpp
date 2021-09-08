#include"Sales_item.h"
#include<iostream>
int main()
{
    Sales_item  total_item;
    if(std::cin >> total_item)
    {
        Sales_item next_item;
        int count = 1;
        while(std::cin >> next_item)
        {
            if(total_item.isbn() == next_item.isbn())
            {
                total_item += next_item;
                count += 1;
            }
            else
            {
                std::cout<<total_item<<" "<<count<<" times"<<std::endl;
                total_item = next_item;
                count = 1;
            }
        }
        std::cout<<total_item<<" "<<count<<" times"<<std::endl;
    }
    else
    {
        std::cout<<"No data"<<std::endl;
        return -1;
    }
    return 0;
}