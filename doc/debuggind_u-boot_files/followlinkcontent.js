/*
*
*  Used to prevent following the link outside content area
*
*  @package CompleteMenuSolution
*  @author Ilya Lebedev <ilya.lebedev.net>
*
*/
CompleteMenuSolution.prototype.modifier.followlinkcontent = {
  runat: 'a',
  /*
  *  modifier itself
  *
  *  @param DOMNode element
  *  @return DOMNode modified element
  *  @access public
  */
  mod : function (el, keys) {
    /*
    *  check the target and stop event, if match span
    *
    *  @param EventTarget
    *  @return boolean
    *  @access protected
    */
    var preventFollowLink = function(e) {
        var el = e.target || e.srcElement
           ,a = getParent(el, 'a');
        if (el.tagName.toLowerCase() == 'a' || document.location.href == a.href) {
          if (e.preventDefault) e.preventDefault();
          e.returnValue = false;
        } else {
          if (e.stopPropagation) e.stopPropagation();
          e.cancelBubble = true;
        }
    }
    /*
    *  prevent submenu close when allowed new page load
    *
    *  @param EventTarget
    *  @return boolean
    *  @access protected
    */
    var preventCloseOnFollow = function(e) {
      var el = e.srcElement || e.target
         ,a  = getParent(el, 'a')
         ,li = getParent(a,  'li'); 
      if (li[keys['openFlag']] && (!el.tagName || el.tagName.toLowerCase() != 'a') && document.location.href != a.href) {
        if (e.stopPropagation) e.stopPropagation();
        e.cancelBubble = true;
      }
    }
    /*
    *  create container and put all A's contents there
    */
    var span = document.createElement('span');
    while (el.firstChild) {
      span.appendChild(el.firstChild);
    }
    el.appendChild(span);
    el.attachEvent('onclick', preventFollowLink);
    el.attachEvent('onmouseup', preventCloseOnFollow);
  }
}