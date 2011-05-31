/*
*  This library appends to DOM interface IE-proprietary extensions
*
*  @version 1.1
*  @title DOMExtensions
*  @author Ilya Lebedev (ilya@lebedev.net), (c) 2004-2005
*  @license GNU LGPL
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*  See http://www.gnu.org/copyleft/lesser.html
*/

if (Node && !Node.prototype) {
  var node = document.createTextNode('');
  var Node = node.constructor;
}
if (window.Node) {
    Node.prototype.removeNode = function(removeChildren) {
    var self = this;
    if (Boolean(removeChildren))
      return this.parentNode.removeChild( self );
    else {
      var r=document.createRange();
      r.selectNodeContents(self);
      return this.parentNode.replaceChild(r.extractContents(),self);
    }
  }
  Node.prototype.swapNode = function(swapNode) {
    var self = this;
    n = self.cloneNode(true);
    nt = swapNode.cloneNode(true);
    self.parentNode.insertBefore(nt,self);
    self.removeNode(true);
    swapNode.parentNode.insertBefore(n,swapNode);
    swapNode.removeNode(true);
  }
  if (!Node.prototype.attachEvent)
    Node.prototype.attachEvent = function (e,f,c) {
      var self = this;
      return self.addEventListener(e.substr(2), f, false); // was true--Opera7b workaround!
    }
  if (!Node.prototype.fireEvent)
    Node.prototype.fireEvent = function (e) {
      var eventTypes = { resize : ['HTMLEvents',1,0],
                       scroll : ['HTMLEvents',1,0],
                       focusin : ['HTMLEvents',0,0],
                       focusout : ['HTMLEvents',0,0],
                       gainselection : ['HTMLEvents',1,0],
                       loseselection : ['HTMLEvents',1,0],
                       activate : ['HTMLEvents',1,1],
                       //events above should be UIEvents, but Mozilla seems does not support this type
                       //or refuses to create such event from inside JS
                       load : ['HTMLEvents',0,0],
                       unload : ['HTMLEvents',0,0],
                       abort : ['HTMLEvents',1,0],
                       error : ['HTMLEvents',1,0],
                       select : ['HTMLEvents',1,0],
                       change : ['HTMLEvents',1,0],
                       submit : ['HTMLEvents',1,1],
                       reset : ['HTMLEvents',1,0],
                       focus : ['HTMLEvents',0,0],
                       blur : ['HTMLEvents',0,0],
                       click : ['MouseEvents',1,1],
                       mousedown : ['MouseEvents',1,1],
                       mouseup : ['MouseEvents',1,1],
                       mouseover : ['MouseEvents',1,1],
                       mousemove : ['MouseEvents',1,0],
                       mouseout : ['MouseEvents',1,0],
                       keypress : ['KeyEvents',1,1],
                       keydown : ['KeyEvents',1,1],
                       keyup : ['KeyEvents',1,1],
                       DOMSubtreeModified : ['MutationEvents',1,0],
                       DOMNodeInserted : ['MutationEvents',1,0],
                       DOMNodeRemoved : ['MutationEvents',1,0],
                       DOMNodeRemovedFromDocument : ['MutationEvents',0,0],
                       DOMNodeInsertedIntoDocument : ['MutationEvents',0,0],
                       DOMAttrModified : ['MutationEvents',1,0],
                       DOMCharacterDataModified : ['MutationEvents',1,0]
                     };
      var self = this;
      e = e.substr(2);
      if (!eventTypes[e]) return false;
      var evt = document.createEvent(eventTypes[e][0]);
      evt.initEvent(e,eventTypes[e][1],eventTypes[e][2]);
      return self.dispatchEvent(evt);
    }
}
if (!window.attachEvent) {
  window.attachEvent= function (e,f,c) {
    var self = this;
    if (self.addEventListener) self.addEventListener(e.substr(2), f, false); // was true--Opera7b workaround!
    else self[e] = f;                                                               // thing for Opera 6......
  }
}