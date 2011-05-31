/*
*  $Id: ajaxum.js 96 2007-02-13 18:12:53Z wingedfox $
*  $HeadURL: https://svn.debugger.ru/repos/CompleteMenuSolution/tags/v0.5.11/modifiers/ajaxum.js $
*
*  Ajaxum is the small modifier to bring Ajax-driven dynamic menus to CMS.
*
*
*  @package CompleteMenuSolution
*  @author Ilya Lebedev <ilya.lebedev.net>
*  @lastmodifier $Author: wingedfox $
*  @title Ajaxum
*  @version $Rev: 96 $
*/

CompleteMenuSolution.prototype.modifier.ajaxum = {
  runat : 'ul',

  menuOptions : {
    ajaxum : {
      'fetcher' : null,
      'error' : {
        'callback' : "Error: no callback method specified",
        'resource' : "Error: no resource specified",
        'fetch' : "Error: could not fetch data from the server"
      },
      'message' : {
        'loading' : "Loading..."
      }
    }
  },
  /*
  *  init function
  *
  *  @param DOMNode element
  *  @param array node keys
  *  @return DOMNode modified element
  *  @access public
  */
  init : function (mo,css,keys) {
    mo.handlers.onOpen.push([this, this.openhandler]);
    keys.ajaxumTarget = '__ajaxumTarget';
  },
  /*
  *  modifier itself
  *
  *  @param DOMNode element
  *  @param array node keys
  *  @return DOMNode modified element
  *  @access public
  */
  mod : function (el, keys, css, mo) {
    if ((!el.firstChild || el.firstChild.nodeType != 8 /* comment */ ) && !el.title)
        el[keys.ajaxumTarget] = null;
    else {
        el[keys.ajaxumTarget] = el.title || el.firstChild.nodeValue.replace(/(^\s*)|(\s*$)/g, "");
        el.innerHTML = "";
        el.title = "";
    }
  },
  /*
  *  performs loading
  *
  *  @param DOMNode currently opened li node
  *  @param hash keys, used on element
  *  @param hash css classes
  *  @param hash menu options
  *  @access protected
  */
  openhandler : function (el, keys, css, mo) {

    el = el[keys.submenu];
    /*
    *  don't process real submenus
    */
    if (el.childNodes.length && !el[keys.ajaxumTarget]) return;

    var showmsg = function (c, msg) {
        el.appendChild(document.createElementExt('span',{'class' : c,
                                                         'child' : [document.createTextNode(msg)]
                                                        }));
    }
    /*
    *  Calback does update of requested element
    *
    *  Hash structure:
    *    state: boolean state of operation
    *    response: string HTML code, part of the tree to be applied
    *
    *  @param Hash object with response from the server
    *  @access protected
    */
    var callback = function (resp) {
      el.innerHTML = "";
      if (!resp.state) {
        showmsg('ajaxum_error',resp.response?"Server Error: "+resp.response:mo.ajaxum.error.fetch);
        return;
      }
      el.innerHTML = resp.response;
      el[keys.ajaxumTarget] = null;

      var root = getParent(el,keys.isRoot,true);
      if (root)
        root[keys['cmsSelf']].reinitSubmenu(el[keys.parentNode]);
    }
    if (!el[keys.ajaxumTarget]) {
      if (null == el[keys.ajaxumTarget]) {
          el.innerHTML = "";
          showmsg('ajaxum_error',mo.ajaxum.error.resource);
      }
      return;
    }
    /*
    *  remove all unwanted stuff from the end
    */
    el.innerHTML = "";
    /*
    *  fetcher should be the function
    */
    if (!mo.ajaxum || !(mo.ajaxum.fetcher instanceof Function)) {
      showmsg('ajaxum_error',mo.ajaxum.error.callback);
      return;
    }
    /*
    *  call fetcher with the specified resource id
    */
    try {
      showmsg('ajaxum_loading',mo.ajaxum.message.loading);
      mo.ajaxum.fetcher(el[keys.ajaxumTarget], callback);
    } catch (e) {
      showmsg('ajaxum_error',mo.ajaxum.error.fetch);
      return;
    }
  }
}