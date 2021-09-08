#include"Sales_item.h"
#include<iostream>
int main()
{
    Sales_item  total_item;
    if(std::cin >> total_item)
    {
        Sales_item next_item;
        while(std::cin >> next_item)
        {
            if(total_item.isbn() == next_item.isbn())
            {
                total_item += next_item;
            }
            else
            {
                std::cout<<total_item<<std::endl;
                total_item = next_item;
                break;
            }
        }
    }
    else
    {
        std::cout<<"No data"<<std::endl;
        return -1;
    }
    return 0;
}