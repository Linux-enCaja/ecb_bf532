/*
*  Count the number of object fields
*
*  @return number of the fields
*  @access public
*/
Object.prototype.length = function () {
   var i = 0;
   for (var k in this) {
      if ('function'!=typeof this.hasOwnProperty || !this.hasOwnProperty(k)) continue;
      i++;
   }
   return i;
}
/*
*  Clone object
*
*  @param optional object to clone
*  @return cloned object
*  @access public
*/
Object.prototype.clone = function (obj) {
   if (typeof(obj) != "object") return obj;
   try { var newObject = new obj.constructor(); } catch(e) {return null;}
   for (var objectItem in obj) {
     if (!obj.hasOwnProperty(objectItem)) continue;
     newObject[objectItem] = obj.clone(obj[objectItem]);
   }
   return newObject;
}

/*
*  Merge objects v0.2
*
*  + fixed merge of arrays
*
*  @param object to merge
*  @param boolean overwrite same keys or no
*  @return void
*  @access public
*/
Object.prototype.merge = function (obj, overwrite) {
  /*
  *  overwrite by default
  */
  try { var n = new obj.constructor(); } catch(e) {return null;}
  try {
    if (isUndefined(overwrite)) overwrite = true;
    for (var i in obj) {
      if (!obj.hasOwnProperty(i)) continue;
      if (isUndefined(this[i]) || (overwrite && typeof this[i] != typeof obj))
        if (obj[i] instanceof Array) this[i] = [];
        else if ('object' == typeof obj[i]) this[i] = {};
      if (obj[i] instanceof Array) this[i] = this[i].concat(obj[i]);
      else if ('object' == typeof obj[i]) this[i].merge(obj[i], overwrite);
      else if (isUndefined(this[i]) || overwrite) this[i] = obj[i];
    }
  } catch(e) {return this}
}


/*
*  Checks if property is derived from prototype, applies method if it is not exists
*
*  @param string property name
*  @return bool true if prototyped
*  @access public
*/
if ('undefined' == typeof Object.hasOwnProperty) {
  Object.prototype.hasOwnProperty = function (prop) {
    return !('undefined' == typeof this[prop] || this.constructor && this.constructor.prototype[prop] && this[prop] === this.constructor.prototype[prop]);
  }
}
