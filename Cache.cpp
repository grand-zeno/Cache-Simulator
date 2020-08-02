//
//  main.cpp
//  nocnwoicn
//
//  Created by Michael Mervin Christy on 28/02/19.
//  Copyright © 2019 Michael Mervin Christy. All rights reserved.
//
#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<set>
#include<queue>
#include<algorithm>
#include<fstream>
#include<random>
using namespace std;
struct cache_block
{
    bool dirty_bit=false;bool valid_bit=false;
    long long int tag=0;
};
class cache
{
private:
    int cache_size,block_size,associativity,replacement_policy,num_sets,num_lines,num_blocks;
    int cache_access,comp_miss,conf_miss,cap_miss,read_access,write_access,read_miss,write_miss,dirty_blocks_evicted;
    vector<vector<cache_block> > grid;
    vector<int> filled;
    vector<vector<int> > PLRU;
    multiset<long long int> seen;
public:
    cache(int a,int b,int c,int d)
    {
        cache_size=a;
        block_size=b;
        associativity=c;
        replacement_policy=d;
        num_blocks=cache_size/block_size;
        comp_miss=0;
        conf_miss=0;
        cap_miss=0;
        cache_access=0;
        read_miss=0;
        write_miss=0;
        read_access=0;
        write_access=0;
        dirty_blocks_evicted=0;
        if(associativity==0)
        {
            num_sets=1;
            num_lines=num_blocks;
        }
        else
        {
            if(associativity==1)
            {
                num_lines=1;
                num_sets=num_blocks;
            }
            else
            {
                num_lines=associativity;
                num_sets=num_blocks/num_lines;
            }
        }
        grid.resize(num_sets);
        filled.resize(num_sets);
        PLRU.resize(num_sets);
        for(int i=0;i<num_sets;i++)
        {
            grid[i].resize(num_lines);
            PLRU[i].resize(num_lines,0);
        }
        for(int i=0;i<num_sets;i++)
        {
            for(int j=0;j<num_lines;j++)
            {
                grid[i][j].dirty_bit=false;
                grid[i][j].valid_bit=false;
            }
        }
    }
    void read(long long int address)
    {
        read_access++;
        cache_access++;
        long long int tag,set_no;
        long long int block_no=address/block_size;
        set_no=block_no%(num_sets);
        tag=block_no/num_sets;
        if(replacement_policy==1)
        {
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==true)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                struct cache_block tempblock=grid[set_no][found_index];
                for(int i=found_index;i<index-1;i++)
                {
                    grid[set_no][i]=grid[set_no][i+1];
                }
                grid[set_no][index-1]=tempblock;
                return;
            }
            else
            {
                read_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //LRU is evicted
                    struct cache_block evicted_block=grid[set_no][0];
                    if(evicted_block.dirty_bit==true)
                    {
                        dirty_blocks_evicted++;
                    }
                    for(int i=0;i<num_lines-1;i++)
                    {
                        grid[set_no][i]=grid[set_no][i+1];
                    }
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=false;
                    newblock.valid_bit=true;
                    grid[set_no][num_lines-1]=newblock;
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    
                    //Cache is not full
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=false;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
            return;
        }
        if(replacement_policy==2)//Pseudo LRU
        {
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==1)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                PLRU[set_no][found_index]=0;
                for(int j=0;j<num_lines;j++)
                {
                    if(j!=found_index)
                    {
                        PLRU[set_no][j]=1;
                    }
                }
            }
            else
            {
                read_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //Pseudo-LRU is evicted
                    for(int i=0;i<num_lines;i++)
                    {
                        if(PLRU[set_no][i]==0)//i is the Most recently used
                        {
                            struct cache_block newblock;
                            newblock.tag=tag;
                            newblock.dirty_bit=false;
                            newblock.valid_bit=true;
                            struct cache_block evicted_block=grid[set_no][(i+1)%num_lines];//The element to the right of the most recently used will be evicted
                            if(evicted_block.dirty_bit==true)
                            {
                                dirty_blocks_evicted++;
                            }
                            grid[set_no][(i+1)%(num_lines)]=newblock;
                            PLRU[set_no][(i+1)%num_lines]=0;
                            PLRU[set_no][i]=1;
                            //return;
                            break;
                        }
                    }
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    //Cache is not full
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=false;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    for(int i=0;i<index;i++)
                    {
                        PLRU[set_no][i]=1;
                    }
                    PLRU[set_no][index]=0;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
            return;
        }
        if(replacement_policy==0)
        {
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==1)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                return;
            }
            else
            {
                read_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //Random block is evicted
                    index=rand()%(num_lines);
                    struct cache_block evicted_block=grid[set_no][index];
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=false;
                    newblock.valid_bit=true;
                    if(evicted_block.dirty_bit==true)
                    {
                        dirty_blocks_evicted++;
                    }
                    grid[set_no][index]=newblock;
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=false;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
        }
    }
    void write(long long int address)
    {
        write_access++;
        cache_access++;
        long long int tag,set_no;
        long long int block_no=address/block_size;
        set_no=block_no%(num_sets);
        tag=block_no/num_sets;
        if(replacement_policy==1)
        {
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==1)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                struct cache_block tempblock=grid[set_no][found_index];
                for(int i=found_index;i<index-1;i++)
                {
                    grid[set_no][i]=grid[set_no][i+1];
                }
                grid[set_no][index-1]=tempblock;
                grid[set_no][index-1].dirty_bit=true;
                return;
            }
            else
            {
                write_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //LRU is evicted
                    struct cache_block evicted_block=grid[set_no][0];
                    if(evicted_block.dirty_bit==true)
                    {
                        dirty_blocks_evicted++;
                    }
                    for(int i=0;i<num_lines-1;i++)
                    {
                        grid[set_no][i]=grid[set_no][i+1];
                    }
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=true;
                    newblock.valid_bit=true;
                    grid[set_no][num_lines-1]=newblock;
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    //Cache is not full
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=true;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
            return;
        }
        if(replacement_policy==2)//Pseudo LRU
        {
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==1)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                PLRU[set_no][found_index]=0;
                for(int j=0;j<num_lines;j++)
                {
                    if(j!=found_index)
                    {
                        PLRU[set_no][j]=1;
                    }
                }
            }
            else
            {
                write_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //Pseudo-LRU is evicted
                    for(int i=0;i<num_lines;i++)
                    {
                        if(PLRU[set_no][i]==0)//i is the Most recently used
                        {
                            struct cache_block newblock;
                            newblock.tag=tag;
                            newblock.dirty_bit=true;
                            newblock.valid_bit=true;
                            struct cache_block evicted_block=grid[set_no][(i+1)%num_lines];
                            if(evicted_block.dirty_bit==true)
                            {
                                dirty_blocks_evicted++;
                            }
                            grid[set_no][(i+1)%(num_lines)]=newblock;
                            PLRU[set_no][(i+1)%num_lines]=0;
                            PLRU[set_no][i]=1;
                            break;
                        }
                    }
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    //Cache is not full
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=true;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    for(int i=0;i<index;i++)
                    {
                        PLRU[set_no][i]=1;
                    }
                    PLRU[set_no][index]=0;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
        }
        if(replacement_policy==0)
        {
            
            int index=0;
            bool isfound=false;
            int found_index=-1;
            while(index<num_lines&&grid[set_no][index].valid_bit==1)
            {
                if(grid[set_no][index].tag==tag)
                {
                    isfound=true;
                    found_index=index;
                }
                index++;
            }
            if(isfound)
            {
                return;
            }
            else
            {
                read_miss++;
                if(index==num_lines)
                {
                    //Cache was full
                    //Random block is evicted
                    index=rand()%(num_lines);
                    struct cache_block evicted_block=grid[set_no][index];
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=true;
                    newblock.valid_bit=true;
                    if(evicted_block.dirty_bit==true)
                    {
                        dirty_blocks_evicted++;
                    }
                    grid[set_no][index]=newblock;
                    if(seen.find(block_no)!=seen.end())
                    {
                        if(associativity==0)
                        {
                            cap_miss++;
                            conf_miss++;
                        }
                        else
                        {
                            conf_miss++;
                        }
                    }
                    else
                    {
                        comp_miss++;
                        seen.insert(block_no);
                    }
                }
                else
                {
                    struct cache_block newblock;
                    newblock.tag=tag;
                    newblock.dirty_bit=true;
                    newblock.valid_bit=true;
                    grid[set_no][index]=newblock;
                    comp_miss++;
                    seen.insert(block_no);
                }
            }
        }
    }
    void output(ofstream &infile)
    {
        infile<<(cache_size)<<endl;
        infile<<(block_size)<<endl;
        infile<<(associativity)<<endl;
        infile<<(replacement_policy)<<endl;
        infile<<(cache_access)<<endl;
        infile<<(read_access)<<endl;
        infile<<(write_access)<<endl;
        infile<<(read_miss+write_miss)<<endl;
        infile<<(comp_miss)<<endl;
        infile<<(conf_miss)<<endl;
        infile<<(cap_miss)<<endl;
        infile<<(read_miss)<<endl;
        infile<<(write_miss)<<endl;
        infile<<(dirty_blocks_evicted)<<endl;
    }
};
pair<long long int,int> parse(string HEX)
{
    long long int ans=0;
    int op=0;
    if(HEX[0]<='9'&&HEX[0]>='0')
    {
        long long int digit=HEX[0]-'0';
        if(digit>=8)
        {
            op=1;
            ans=digit-8;
        }
        else
        {
            op=0;
            ans=digit;
        }
    }
    else
    {
        long long int digit=HEX[0]-'A'+10;
        if(digit>=8)
        {
            op=1;
            ans=digit-8;
        }
        else
        {
            op=0;
            ans=digit;
        }
    }
    for(int i=1;i<8;i++)
    {
        if(HEX[i]<='9'&&HEX[i]>='0')
        {
            ans=ans*(16)+(HEX[i]-'0');
        }
        else
        {
            ans=ans*(16)+(HEX[i]-'A'+10);
        }
    }
    return make_pair(ans,op);
}
int main()
{
    srand(time(NULL));
    ifstream Infile;
    Infile.open("input.txt");
    ofstream Outfile;
    Outfile.open("output.txt");
    int a,b,c,d;
    Infile>>a>>b>>c>>d;
    cache Map(a,b,c,d);
    while(!Infile.eof())
    {
        string address;
        Infile>>address;
        long long int add;int op;
        if(address=="")
        {
            break;
        }
        pair<long long int,int> res=parse(address);
        add=res.first;
        op=res.second;
        if(op==0)
        {
            Map.read(add);
        }
        else
        {
            Map.write(add);
        }
    }
    Map.output(Outfile);
}



