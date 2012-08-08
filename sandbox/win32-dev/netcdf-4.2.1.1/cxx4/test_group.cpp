// Purpose: Converts ida3 format xma data to netcdf4
// Usage:   xma2netcdf <shot number>


#include <iostream>
#include <ncFile.h>
#include <ncException.h>
#include <iomanip>
#include <netcdf>
using namespace std;
using namespace netCDF;
using namespace netCDF::exceptions;


int main()
{
try
  {
    cout<<"Opening file \"firstFile.cdf\" with NcFile::replace"<<endl;
    NcFile ncFile("firstFile.cdf",NcFile::replace);
    
    cout<<left<<setw(50)<<"Testing addGroup(\"groupName\")";
    NcGroup groupA(ncFile.addGroup("groupA"));
    NcGroup groupA0(ncFile.addGroup("groupA0"));
    NcGroup groupB(groupA.addGroup("groupB"));
    NcGroup groupC(groupA.addGroup("groupC"));
    cout <<"    -----------   passed\n";
    
    cout <<left<<setw(50)<<"Testing getGroupCount([netCDF::Location])";
    if( ncFile.getGroupCount()!= 2)                                 throw NcException("NcException","Error in test 1.1",__FILE__,__LINE__);
    if( groupA.getGroupCount()!= 2)                                 throw NcException("NcException","Error in test 1.2",__FILE__,__LINE__);
    if( groupB.getGroupCount() !=0)                                 throw NcException("NcException","Error in test 1.3",__FILE__,__LINE__);
    if( ncFile.getGroupCount(NcGroup::AllGrps) != 5)                throw NcException("NcException","Error in test 1.4",__FILE__,__LINE__);
    if( ncFile.getGroupCount(NcGroup::AllChildrenGrps) != 4)        throw NcException("NcException","Error in test 1.5",__FILE__,__LINE__);
    if( groupA.getGroupCount(NcGroup::AllGrps) != 4)                throw NcException("NcException","Error in test 1.6",__FILE__,__LINE__);
    if( groupB.getGroupCount(NcGroup::ParentsGrps) != 2)            throw NcException("NcException","Error in test 1.7",__FILE__,__LINE__);
    if( groupB.getGroupCount(NcGroup::ChildrenGrps) != 0)           throw NcException("NcException","Error in test 1.8",__FILE__,__LINE__);
    if( groupB.getGroupCount(NcGroup::ChildrenOfChildrenGrps) != 0) throw NcException("NcException","Error in test 1.9",__FILE__,__LINE__);
    if( groupB.getGroupCount(NcGroup::ParentsAndCurrentGrps) != 3)  throw NcException("NcException","Error in test 1.10",__FILE__,__LINE__);
    if( groupB.getGroupCount(NcGroup::AllChildrenGrps) != 0)        throw NcException("NcException","Error in test 1.11",__FILE__,__LINE__);
    cout <<"    -----------   passed\n";


    cout <<left<<setw(50)<<"Testing getGroups([netCDF::Location])";
    {

      multimap<string,NcGroup> groupMap;
      multimap<string,NcGroup>::iterator iter;

      // operations on ncFile

      groupMap = ncFile.getGroups();
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.1",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.2",__FILE__,__LINE__);
      iter=groupMap.find("groupA0");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.3",__FILE__,__LINE__);
      
      groupMap =ncFile.getGroups(NcGroup::ChildrenGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.4",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.5",__FILE__,__LINE__);
      iter=groupMap.find("groupA0");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.6",__FILE__,__LINE__);
      
      groupMap = ncFile.getGroups(NcGroup::ParentsGrps);
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.7",__FILE__,__LINE__);

      groupMap = ncFile.getGroups(NcGroup::ChildrenOfChildrenGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.8",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.9",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.10",__FILE__,__LINE__);

      groupMap = ncFile.getGroups(NcGroup::AllChildrenGrps);
      if( groupMap.size() != 4)      throw NcException("NcException","Error in test 2.11",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.12",__FILE__,__LINE__);
      iter=groupMap.find("groupA0");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.13",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.14",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.15",__FILE__,__LINE__);
      
      groupMap = ncFile.getGroups(NcGroup::ParentsAndCurrentGrps);
      if( groupMap.size() != 1)      throw NcException("NcException","Error in test 2.16",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.17",__FILE__,__LINE__);

      groupMap = ncFile.getGroups(NcGroup::AllGrps);
      if( groupMap.size() != 5)      throw NcException("NcException","Error in test 2.18",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.19",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.20",__FILE__,__LINE__);
      iter=groupMap.find("groupA0");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.21",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.22",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.23",__FILE__,__LINE__);


      // operations on groupA

      groupMap = groupA.getGroups();
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.24",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.25",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.26",__FILE__,__LINE__);
      

      groupMap =groupA.getGroups(NcGroup::ChildrenGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.27",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.28",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.29",__FILE__,__LINE__);
      
      groupMap = groupA.getGroups(NcGroup::ParentsGrps);
      if( groupMap.size() != 1)      throw NcException("NcException","Error in test 2.30",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.31",__FILE__,__LINE__);

      groupMap = groupA.getGroups(NcGroup::ChildrenOfChildrenGrps);
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.32",__FILE__,__LINE__);

      groupMap = groupA.getGroups(NcGroup::AllChildrenGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.33",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.34",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.35",__FILE__,__LINE__);
      
      groupMap = groupA.getGroups(NcGroup::ParentsAndCurrentGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.36",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.37",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.38",__FILE__,__LINE__);

      groupMap = groupA.getGroups(NcGroup::AllGrps);
      if( groupMap.size() != 4)      throw NcException("NcException","Error in test 2.39",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.40",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.41",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.42",__FILE__,__LINE__);
      iter=groupMap.find("groupC");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.43",__FILE__,__LINE__);

      // operations on groupB

      groupMap = groupB.getGroups();
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.44",__FILE__,__LINE__);

      groupMap =groupB.getGroups(NcGroup::ChildrenGrps);
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.45",__FILE__,__LINE__);
      
      groupMap = groupB.getGroups(NcGroup::ParentsGrps);
      if( groupMap.size() != 2)      throw NcException("NcException","Error in test 2.46",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.47",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.48",__FILE__,__LINE__);

      groupMap = groupB.getGroups(NcGroup::ChildrenOfChildrenGrps);
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.49",__FILE__,__LINE__);

      groupMap = groupB.getGroups(NcGroup::AllChildrenGrps);
      if( groupMap.size() != 0)      throw NcException("NcException","Error in test 2.50",__FILE__,__LINE__);
      
      groupMap = groupB.getGroups(NcGroup::ParentsAndCurrentGrps);
      if( groupMap.size() != 3)      throw NcException("NcException","Error in test 2.51",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.52",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.53",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.54",__FILE__,__LINE__);

      groupMap = groupB.getGroups(NcGroup::AllGrps);
      if( groupMap.size() != 3)      throw NcException("NcException","Error in test 2.55",__FILE__,__LINE__);
      iter=groupMap.find("/");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.56",__FILE__,__LINE__);
      iter=groupMap.find("groupA");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.57",__FILE__,__LINE__);
      iter=groupMap.find("groupB");
      if( iter == groupMap.end())    throw NcException("NcException","Error in test 2.58",__FILE__,__LINE__);

    }
    cout <<"    -----------   passed\n";



    cout <<left<<setw(50)<<"Testing getGroups(\"name\",[netCDF::Location])";
    {

      set<NcGroup> groupSet;
      set<NcGroup>::iterator iter;

      // operations on ncFile:groupA

      groupSet = ncFile.getGroups("groupA");
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.1",__FILE__,__LINE__);
      iter=groupSet.find(groupA);
      if( iter == groupSet.end())    throw NcException("NcException","Error in test 3.2",__FILE__,__LINE__);
      if( iter->getName() != "groupA")throw NcException("NcException","Error in test 3.3",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::ChildrenGrps);
      if( groupSet.count(groupA) != 1)throw NcException("NcException","Error in test 3.4",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::ParentsGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.5",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.6",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.7",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.8",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupA",NcGroup::AllGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.9",__FILE__,__LINE__);
      

      // operations on ncFile:groupB

      groupSet = ncFile.getGroups("groupB");
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.10",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::ChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.11",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::ParentsGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.12",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.13",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.14",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.15",__FILE__,__LINE__);

      groupSet = ncFile.getGroups("groupB",NcGroup::AllGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.16",__FILE__,__LINE__);
      

      // operations on groupA:groupA

      groupSet = groupA.getGroups("groupA");
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.17",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::ChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.18",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::ParentsGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.19",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.20",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.21",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.22",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA",NcGroup::AllGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.23",__FILE__,__LINE__);
      


      // operations on groupA:ncFile

      groupSet = groupA.getGroups("/");
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.24",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::ChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.25",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::ParentsGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.26",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.27",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.28",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.29",__FILE__,__LINE__);

      groupSet = groupA.getGroups("/",NcGroup::AllGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.30",__FILE__,__LINE__);
      


      // operations on groupA:groupB

      groupSet = groupA.getGroups("groupB");
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.31",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::ChildrenGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.32",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::ParentsGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.33",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.34",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.35",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.36",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupB",NcGroup::AllGrps);
      if( groupSet.size() != 1)      throw NcException("NcException","Error in test 3.37",__FILE__,__LINE__);
      

      // operations on groupA:groupA0

      groupSet = groupA.getGroups("groupA0");
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.38",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::ChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.39",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::ParentsGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.40",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::ChildrenOfChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.41",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::AllChildrenGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.42",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::ParentsAndCurrentGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.43",__FILE__,__LINE__);

      groupSet = groupA.getGroups("groupA0",NcGroup::AllGrps);
      if( groupSet.size() != 0)      throw NcException("NcException","Error in test 3.44",__FILE__,__LINE__);
      
    }

    cout <<"    -----------   passed\n";
    
    
    


    cout <<left<<setw(50)<<"Testing getGroup(\"name\",[netCDF::Location])";
    {

      // operations on ncFile:groupA

      if( ncFile.getGroup("groupA").getName() != "groupA")  throw NcException("NcException","Error in test 4.1",__FILE__,__LINE__);
      if( !ncFile.getGroup("groupB").isNull())              throw NcException("NcException","Error in test 4.2",__FILE__,__LINE__);
      
    }

    cout <<"    -----------   passed\n";
    



    cout <<left<<setw(50)<<"Testing getParentGroup()";

    if( !ncFile.getParentGroup().isNull())            throw NcException("NcException","Error in test 5.1",__FILE__,__LINE__);
    if( groupA.getParentGroup().getName() != "/")     throw NcException("NcException","Error in test 5.2",__FILE__,__LINE__);
    if( groupA0.getParentGroup().getName() != "/")    throw NcException("NcException","Error in test 5.3",__FILE__,__LINE__);
    if( groupB.getParentGroup().getName() != "groupA")throw NcException("NcException","Error in test 5.4",__FILE__,__LINE__);
    if( groupC.getParentGroup().getName() != "groupA")throw NcException("NcException","Error in test 5.5",__FILE__,__LINE__);
    
    cout <<"    -----------   passed\n";


  }
 catch (NcException& e)
   {
     cout << "unknown error"<<endl;
     e.what();
   }
}
