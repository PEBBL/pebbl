#! /usr/bin/env python
##
## A python script that reads an XML input file, computes a simple function, 
## and writes out an XML file.
##
## Note: this script can be customized for new applications.  The CUSTOMIZE HERE 
## notes indicate the parts of this script that would need to be edited.
##

import sys
import xml.dom.minidom
import re

#
# The type of parameters supported by this application
#
# CUSTOMIZE HERE
#
class MixedIntVars(object):
  def __init__(self):
      self.reals = []
      self.ints = []
      self.bits = []

  def display(self):
      print "Reals",
      for val in self.reals:
	print val,
      print ""
      print "Integers",
      for val in self.ints:
	print val,
      print ""
      print "Binary",
      for val in self.bits:
	print val,
      print ""

#
# The test function
#
# CUSTOMIZE HERE
#
def func(mivars):
  val=0.0
  for i in range(0,len(mivars.reals)):
    val = (i+1)*mivars.reals[i]
  for i in range(0,len(mivars.ints)):
    val = 1000*(i+1)*mivars.ints[i]
  for i in range(0,len(mivars.bits)):
    val = 1000000*(i+1)*mivars.bits[i]
  return val

#
# The test gradient function
#
# CUSTOMIZE HERE
#
def gfunc(mivars):
  val = []
  for i in range(0,len(mivars.reals)):
    val = val + [ (i+1) ]
  return val

#
# The test constraint function
#
# CUSTOMIZE HERE
#
def cfunc(mivars):
  val = []
  for i in range(0,len(mivars.reals)):
    val = val + [(i+1) + mivars.reals[i]*mivars.reals[i]]
  for i in range(0,len(mivars.ints)):
    val = val + [(i+1) + 1000*mivars.reals[i]*mivars.reals[i]]
  for i in range(0,len(mivars.bits)):
    val = val + [(i+1) + 1000000*mivars.reals[i]*mivars.reals[i]]
  return val


#
# Get text for a node
#
def get_text(node):
  nodetext = ""
  for child in node.childNodes:
    if child.nodeType == node.TEXT_NODE:
       nodetext = nodetext + child.nodeValue
  nodetext = str(nodetext)
  return nodetext.strip()


#
# A function that processes the Parameters element
#
# CUSTOMIZE HERE
#
def handleParameters(node):
  mivars = MixedIntVars()
  for child in node.childNodes:
    if child.nodeType == node.ELEMENT_NODE:
       child_text = get_text(child)
       if child_text == "":
          continue
       if child.nodeName == "Real":
          for val in re.split('[\t ]+',child_text):
            mivars.reals.append(1.0*eval(val))
       if child.nodeName == "Integer":
          for val in re.split('[\t ]+',child_text):
            mivars.ints.append(eval(val))
       if child.nodeName == "Binary":
          for val in child_text:
	    if val == '1':
               mivars.bits.append(1)
	    elif val == '0':
               mivars.bits.append(0)
  return mivars

#
# A function that processes the requests
#
def handleRequests(node):
  requests = {}
  for child in node.childNodes:
    if child.nodeType == node.ELEMENT_NODE:
       tmp = {}
       for (name,value) in child.attributes.items():
	 tmp[name]=value
       requests[str(child.nodeName)] = tmp
  return requests

#
# The main routine to parse the input file
#
def handleColinRequest(doc):
  point = handleParameters(doc.getElementsByTagName("Domain")[0])
  requests = handleRequests(doc.getElementsByTagName("Requests")[0])
  return [point,requests]

#
# A utility function to create strings from arrays of numbers
#
def tostr(array):
  tmpstr = ""
  for val in array:
    tmpstr = tmpstr + " " + `val`
  return tmpstr

#
# Process the document
#
# CUSTOMIZE HERE
#
def process(point,requests):
  #
  # Setup document
  #
  doc = xml.dom.minidom.Document()
  root = doc.createElement("ColinResponse")
  doc.appendChild(root)
  for key in requests.keys():
    if key == "FunctionValue":
       elt = doc.createElement(key)
       root.appendChild(elt)
       ans = func(point)
       text_elt = doc.createTextNode( str(ans) )
       elt.appendChild(text_elt)
    elif key == "ConstraintValues":
       elt = doc.createElement(key)
       root.appendChild(elt)
       ans = cfunc(point)
       text_elt = doc.createTextNode( tostr(ans) )
       elt.appendChild(text_elt)
    elif key == "Gradient":
       elt = doc.createElement(key)
       root.appendChild(elt)
       ans = gfunc(point)
       text_elt = doc.createTextNode( tostr(ans) )
       elt.appendChild(text_elt)
    else:
       elt = doc.createElement(key)
       root.appendChild(elt)
       text_elt = doc.createTextNode( "ERROR: Unsupported application request" )
       elt.appendChild(text_elt)
  return doc
       
#
# MAIN
#
if len(sys.argv) < 3:
   print "testfunc.py <input> <output> <log>"
   sys.exit(1)
#
# Parse XML input file
#
input_doc = xml.dom.minidom.parse(sys.argv[1])
[point,requests] = handleColinRequest(input_doc)
#
# Create output XML object
#
output_doc = process(point,requests)
OUTPUT = open(sys.argv[2],"w")
output_doc.writexml(OUTPUT," "," ","\n","UTF-8")
OUTPUT.close()
