#include<iostream>
#include "Sales_data.h"
int main()
{
    Sales_data total;
    double price = 0.0;
    if(std::cin>>total.bookNo>>total.units_sold>>price)
    {
        total.revenue = total.units_sold * price;
        Sales_data next_data;
        while(std::cin>>next_data.bookNo>>next_data.units_sold>>price)
        {
            next_data.revenue = next_data.units_sold * price;
            if(total.bookNo == next_data.bookNo)
            {
                total.units_sold += next_data.units_sold;
                total.revenue += next_data.revenue;
            }
            else
            {
                std::cout<<total.bookNo<<" "<<total.units_sold<<" "<<total.revenue<<" ";
                if(total.units_sold != 0)
                {
                    std::cout<<total.revenue/total.units_sold<<std::endl;
                }
                else
                {
                    std::cout<<" no sale"<<std::endl;
                }
                total.bookNo = next_data.bookNo;
                total.units_sold = next_data.units_sold;
                total.revenue = next_data.revenue;

            }
        }
        std::cout<<total.bookNo<<" "<<total.units_sold<<" "<<total.revenue<<" ";
        if(total.units_sold != 0)
        {
            std::cout<<total.revenue/total.units_sold<<std::endl;
        }
        else
        {
            std::cout<<" no sale"<<std::endl;
        }

        
    }
    else
    {
        std::cout<<"no data"<<std::endl;
        return -1;
    }
    return 0;
}