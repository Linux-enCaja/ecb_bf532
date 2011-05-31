/**
 *  $Id: helpers.js 130 2006-11-30 20:44:53Z wingedfox $
 *  $HeadURL: https://svn.debugger.ru/repos/jslibs/BrowserExtensions/tags/BrowserExtensions.001/helpers.js $
 *
 *  File contains differrent helper functions
 * 
 * @author Ilya Lebedev <ilya@lebedev.net>
 * @license LGPL
 * @version $Rev: 130 $
 */
//-----------------------------------------------------------------------------
//  Variable/property checks
//-----------------------------------------------------------------------------
/**
 *  Checks if property is undefined
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isUndefined (prop /* :Object */) /* :Boolean */ {
  return (typeof prop == 'undefined');
}
/**
 *  Checks if property is function
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isFunction (prop /* :Object */) /* :Boolean */ {
  return (typeof prop == 'function');
}
/**
 *  Checks if property is string
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isString (prop /* :Object */) /* :Boolean */ {
  return (typeof prop == 'string');
}
/**
 *  Checks if property is number
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isNumber (prop /* :Object */) /* :Boolean */ {
  return (typeof prop == 'number');
}
/**
 *  Checks if property is the calculable number
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isNumeric (prop /* :Object */) /* :Boolean */ {
  return isNumber(prop)&&!isNaN(prop)&&isFinite(prop);
}
/**
 *  Checks if property is array
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isArray (prop /* :Object */) /* :Boolean */ {
  return (prop instanceof Array);
}
/**
 *  Checks if property is regexp
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isRegExp (prop /* :Object */) /* :Boolean */ {
  return (prop instanceof RegExp);
}
/**
 *  Checks if property is a boolean value
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isBoolean (prop /* :Object */) /* :Boolean */ {
  return ('boolean' == typeof prop);
}
/**
 *  Checks if property is a scalar value (value that could be used as the hash key)
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isScalar (prop /* :Object */) /* :Boolean */ {
  return isNumeric(prop)||isString(prop);
}
/**
 *  Checks if property is empty
 *
 *  @param {Object} prop value to check
 *  @return {Boolean} true if matched
 *  @scope public
 */
function isEmpty (prop /* :Object */) /* :Boolean */ {
  if (isBoolean(prop)) return false;
  if (isRegExp(prop) && new RegExp("").toString() == prop.toString()) return true;
  if (isString(prop) || isNumber(prop)) return !prop;
  if (Boolean(prop)&&false != prop) {
    for (var i in prop) if(prop.hasOwnProperty(i)) return false
  }
  return true;
}

function dump (v, l) {
    var s = [];
    if (!l) l=0;
    if (l>2) return "***********8Recursion*************";
    var sp = "";
    for (var i=0;i<l;i++) sp+="    ";
    for (var i in v) {
//        if (!v.hasOwnProperty(i)) continue;
        var q = [sp,i,': '];
        try {
        if (!isScalar(v[i]) && !isFunction(v[i])) {
            q[q.length] = '{';
            s[s.length] = q.join("");            
            s[s.length] = dump(v[i],l+1);
            s[s.length] = '}';
        } else {
            q[q.length] = v[i];            
            s[s.length] = q.join(""); 
        }
        } catch (err) {} 
    }
    return s.join("\n");
} 
//-----------------------------------------------------------------------------
//  File paths functions
//-----------------------------------------------------------------------------
/**
 *  used to glue path's
 *
 *  @param {String} number of strings
 *  @return {String} glued path
 *  @scope public
 */
function gluePath () /* :String */ {
  var aL=arguments.length, i=aL-2, s = arguments[aL-1]; 
  for(;i>=0;i--)
    s = ((!isString(arguments[i])&&!isNumber(arguments[i]))||isEmpty(arguments[i])
        ?s
        :arguments[i]+'\x00'+s); 
  return s?s.replace(/\/*\x00+\/*/g,"/"):"";
}

/**
 *  return full path to the script
 *
 *  @param {String} sname script name
 *  @return {String, Null} mixed string full path or null
 *  @scope public
 */
function findPath (sname /* :String */) /* :String */{
  var sc = document.getElementsByTagName('script'),
      sr = new RegExp('^(.*/|)('+sname+')([#?]|$)');
  for (var i=0,scL=sc.length; i<scL; i++) {
    // matched desired script
    var m = String(sc[i].src).match(sr);
    if (m) {
      /*
      *  we've matched the full path
      */
      if (m[1].match(/^((https?|file)\:\/{2,}|\w:[\\])/)) return m[1];
      /*
      *  we've matched absolute path from the site root
      */
      if (m[1].indexOf("/")==0) return m[1];
      b = document.getElementsByTagName('base');
      if (b[0] && b[0].href) return b[0].href+m[1];
      /*
      *  return matching part of the document location and path to js file
      */
      return (document.location.pathname.match(/(.*[\/\\])/)[0]+m[1]).replace(/^\/+(?=\w:)/,"");
    }
  }
  return null;
}

//-----------------------------------------------------------------------------
//  DOM related stuff
//-----------------------------------------------------------------------------
/**
 *  Performs parent lookup by
 *   - node object: actually it's "is child of" check
 *   - tagname: getParent(el, 'li') == getParent(el, 'tagName', 'LI')
 *   - any node attribute
 *
 *  @param {HTMLElement} el source element
 *  @param {HTMLElement, String} cp DOMNode or string tagname or string attribute name
 *  @param {String} vl optional attribute value
 *  @return {HTMLElement, Null}
 *  @scope public
 */
function getParent (el /* : HTMLElement */, cp /* :String, HTMLElement */, vl /* :String */) /* :HTMLElement */ {
  if (el == null) return null; else
  if (el.nodeType == 1 &&
      ((!isUndefined(vl) && el[cp] == vl) ||
       ('string' == typeof cp && hasTagName(el, cp)) ||
       el == cp)) return el;
  else return getParent(el.parentNode, cp, vl); 
}
/**
 *  Checks, if property matches a tagname(s)
 * 
 *  @param {HTMLElement} prop
 *  @param {String, Array} tags
 *  @return {Boolean}
 *  @scope public
 */
function hasTagName (prop /* :HTMLElement */, tags /* :String, Array */) {
    if (isString(tags)) tags = [tags];
    if (!isArray(tags) || isEmpty(tags) || isUndefined(prop) || isEmpty(prop.tagName)) return false;
    var t = prop.tagName.toLowerCase();
    for (var i=0, tL=tags.length; i<tL; i++) {
        if (tags[i].toLowerCase() == t) return true;
    }
    return false;
}
/**
 *  Method is used to convert table into the array
 *  
 * @param {String, HTMLTableElement, HTMLTBodyElement, HTMLTHeadElement, HTMLTFootElement} id
 * @param {Number} ci column indexes to put in the array
 * @param {String} section optional section type
 * @param {Object} subsection optional subsection index
 * @return {NULL, Array}
 * @scope public
 */
function table2array (id, ci, section, subsection) {
    if (isString(id)) id = document.getElementById(id);
    if (!id || !hasTagName(id, ['table','tbody,','thead','tfoot'])) return null;
    if (!isEmpty(section) && (!isString(section) || !(id = id.getElementsByTagName(section)))) return null;
    if (!isEmpty(subsection) && (!isNumber(subsection) || subsection<0 || !(id = id[subsection]))) return null;
    if (isUndefined(id.rows)) return null;

    var res = [];
    for (var i=0, rL=id.rows.length; i<rL; i++) {
        var tr = [];
        if (isArray(ci)) {
            for (var o=0, cL=ci.length; o<cL; o++) 
                if (id.rows[i].cells[ci[o]]) tr[tr.length] = id.rows[i].cells[ci[o]].innerHTML.replace(/<[^>]*?>/g,"");
                
        } else {
            for (var z=0, tL=id.rows[i].cells.length; z<tL; z++) 
                tr[tr.length] = id.rows[i].cells[z].innerHTML.replace(/<[^>]*?>/g,"");
        }   
        if (!isEmpty(tr)) res[res.length] = tr;
    }
    return res;
}


/**
 *  Creates element all-at-once
 *
 *  @param {String} tag name
 *  @param {Object} p element properties { 'class' : 'className',
 *                                         'style' : { 'property' : value, ... },
 *                                         'event' : { 'eventType' : handler, ... },
 *                                         'child' : [ child1, child2, ...],
 *                                         'param' : { 'property' : value, ... },
 *  @return {HTMLElement} created element or null
 *  @scope public
 */
document.createElementExt = function (tag /* :String */, p /* :Object */ ) /* :HTMLElement */{
  var L, i, k, el = document.createElement(tag);
  if (!el) return null;
  for (i in p) {
    if (!p.hasOwnProperty(i)) continue;
    switch (i) {
      case "class" : el.setAttribute('className',p[i]); el.setAttribute('class',p[i]); break;
      case "style" : for (k in p[i]) { if (!p[i].hasOwnProperty(k)) continue; el.style[k] = p[i][k]; } break;
      case "event" : for (k in p[i]) { if (!p[i].hasOwnProperty(k)) continue; el.attachEvent(k,p[i][k]); } break;
      case "child" : L = p[i].length; for (k = 0; k<L; k++) el.appendChild(p[i][k]); break;
      case "param" : for (k in p[i]) { if (!p[i].hasOwnProperty(k)) continue; try { el[k] = p[i][k] } catch(e) {} } break;
    }
  }
  return el;
}
//-----------------------------------------------------------------------------
//  Misc helpers
//-----------------------------------------------------------------------------
/**
 * simple setInterval/setTimout wrappers
 *
 * @param {Function} f function to be launched
 * @param {Number} i interval
 * @param {Array} o optional function parameters to be applied
 * @return {Number} interval id
 * @scope public
 */
function playInterval (f /* :Function */, i /* :Number */, o /* :Array */) /* :Number */ { return setInterval(function(){(o instanceof Array)?f.apply(this,o):f.call(this,o)},i) }
function playTimeout (f /* :Function */, i /* :Number */, o /* :Array */) /* :Number */ { return setTimeout(function(){(o instanceof Array)?f.apply(this,o):f.call(this,o)},i) }
