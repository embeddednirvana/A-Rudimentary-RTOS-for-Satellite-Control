###################################################################################### 
#	 Copyright 2011 Nishchay Mhatre 

#	Licensed under the Apache License, Version 2.0 (the "License");
#	you may not use this file except in compliance with the License.
#	You may obtain a copy of the License at
#
#	http://www.apache.org/licenses/LICENSE-2.0
#
#	Unless required by applicable law or agreed to in writing, software
#	distributed under the License is distributed on an "AS IS" BASIS,
#	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#	See the License for the specific language governing permissions and
#	limitations under the License.

##################################################################################### */ 

# Arguments :
# 1 -> Input elf file.
# 2 -> Input bin file.
# 3 -> Output bin file with the isolated section.

phy_start=`readelf -a $1 | egrep "LOAD" | head -n 1 | cut -d " " -f 16`
phy_text=`readelf -a $1 | egrep "LOAD" | head -n 3 | tail -n 1  | cut -d " " -f 16`
phy_size=`readelf -a $1 | egrep "LOAD" | head -n 3 | tail -n 1  | cut -d " " -f 18`

start=`python -c "print int('$phy_text',16)-int('$phy_start',16)"`
echo $start
size_new=`python -c "print int('$phy_size',16)"`
echo $phy_size
echo $size_new 
echo $2 $3
# Call the file which will isolate the part mentioned. 
./ielf $2 $start $3 $size_new
