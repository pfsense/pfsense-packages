

/**************************************************************

	Script		: Sortable Table
	Version		: 1.4
	Authors		: Samuel Birch
	Desc			: Sorts and filters table elements
	Licence		: Open Source MIT Licence

**************************************************************/

var sortableTable = new Class({
							  
	getOptions: function(){
		return {
			overCls: false,
			onClick: false,
			sortOn: 0,
			sortBy: 'ASC',
			filterHide: true,
			filterHideCls: 'hide',
			filterSelectedCls: 'selected'
		};
	},

	initialize: function(table, options){
		this.setOptions(this.getOptions(), options);
		this.table = $(table);
		this.tHead = this.table.getElement('thead');
		this.tBody = this.table.getElement('tbody');
		this.tFoot = this.table.getElement('tfoot');
		this.elements = this.tBody.getElements('tr');
		this.filtered = false;
		
		/*for(i=0;i<10;i++){
			this.elements.clone().injectInside(this.tBody);
		}
		this.elements = this.tBody.getElements('tr');*/
		
		this.elements.each(function(el,i){
			if(this.options.overCls){
				el.addEvent('mouseover', function(){
					el.addClass(options.overCls);
				}, this);
				el.addEvent('mouseout', function(){
					el.removeClass(options.overCls);
				});
			}
			if(this.options.onClick){
				el.addEvent('click', options.onClick);
			}
		}, this);
		
		//setup header
		this.tHead.getElements('th').each(function(el,i){
			if(el.axis){
				el.addEvent('click', this.sort.bind(this,i));
				el.addEvent('mouseover', function(){
					el.addClass('tableHeaderOver');
				});
				el.addEvent('mouseout', function(){
					el.removeClass('tableHeaderOver');
				});
				el.getdate = function(str){
					// inner util function to convert 2-digit years to 4
					function fixYear(yr) {
						yr = +yr;
						if (yr<50) { yr += 2000; }
						else if (yr<100) { yr += 1900; }
						return yr;
					};
					var ret;
					//
					if (str.length>12){
						strtime = str.substring(str.lastIndexOf(' ')+1);
						strtime = strtime.substring(0,2)+strtime.substr(-2)
					}else{
						strtime = '0000';
					}
					//
					// YYYY-MM-DD
					if (ret=str.match(/(\d{2,4})-(\d{1,2})-(\d{1,2})/)) {
						return (fixYear(ret[1])*10000) + (ret[2]*100) + (+ret[3]) + strtime;
					}
					// DD/MM/YY[YY] or DD-MM-YY[YY]
					if (ret=str.match(/(\d{1,2})[\/-](\d{1,2})[\/-](\d{2,4})/)) {
						return (fixYear(ret[3])*10000) + (ret[2]*100) + (+ret[1]) + strtime;
					}
					return 999999990000; // So non-parsed dates will be last, not first
				};
				//
				el.findData = function(elem){
					var child = elem.getFirst();
					if(child){
						return el.findData(child);
					}else{
						return elem.innerHTML.trim();
					}
				};
				//
				el.compare = function(a,b){
					var1 = el.findData(a.getChildren()[i]);
					var2 = el.findData(b.getChildren()[i]);
					//var1 = a.getChildren()[i].firstChild.data;
					//var2 = b.getChildren()[i].firstChild.data;
					
					if(el.axis == 'number'){
						var1 = parseFloat(var1);
						var2 = parseFloat(var2);
						
						if(el.sortBy == 'ASC'){
							return var1-var2;
						}else{
							return var2-var1;
						}
						
					}else if(el.axis == 'string'){
						var1 = var1.toUpperCase();
						var2 = var2.toUpperCase();
						
						if(var1==var2){return 0};
						if(el.sortBy == 'ASC'){
							if(var1<var2){return -1};
						}else{
							if(var1>var2){return -1};
						}
						return 1;
						
					}else if(el.axis == 'date'){
						var1 = parseFloat(el.getdate(var1));
						var2 = parseFloat(el.getdate(var2));
						
						if(el.sortBy == 'ASC'){
							return var1-var2;
						}else{
							return var2-var1;
						}
						
					}else if(el.axis == 'currency'){
						var1 = parseFloat(var1.substr(1).replace(',',''));
						var2 = parseFloat(var2.substr(1).replace(',',''));
						
						if(el.sortBy == 'ASC'){
							return var1-var2;
						}else{
							return var2-var1;
						}
						
					}
					
				}
				
				if(i == this.options.sortOn){
					el.fireEvent('click');
				}
			}
		}, this);
	},
	
	sort: function(index){
		if(this.options.onStart){
			this.fireEvent('onStart');
		}
		//
		this.options.sortOn = index;
		var header = this.tHead.getElements('th');
		var el = header[index];
		
		header.each(function(e,i){
			if(i != index){
				e.removeClass('sortedASC');
				e.removeClass('sortedDESC');
			}
		});
		
		if(el.hasClass('sortedASC')){
			el.removeClass('sortedASC');
			el.addClass('sortedDESC');
			el.sortBy = 'DESC';
		}else if(el.hasClass('sortedDESC')){
			el.removeClass('sortedDESC');
			el.addClass('sortedASC');
			el.sortBy = 'ASC';
		}else{
			if(this.options.sortBy == 'ASC'){
				el.addClass('sortedASC');
				el.sortBy = 'ASC';
			}else if(this.options.sortBy == 'DESC'){
				el.addClass('sortedDESC');
				el.sortBy = 'DESC';
			}
		}
		//
		this.elements.sort(el.compare);
		this.elements.injectInside(this.tBody);
		//
		if(this.filtered){
			this.filteredAltRow();
		}else{
			this.altRow();
		}
		
		//
		if(this.options.onComplete){
			this.fireEvent('onComplete');
		}
	},
	
	altRow: function(){
		this.elements.each(function(el,i){
			if(i % 2){
				el.removeClass('altRow');
			}else{
				el.addClass('altRow');
			}
		});
	},
	
	filteredAltRow: function(){
		this.table.getElements('.'+this.options.filterSelectedCls).each(function(el,i){
			if(i % 2){
				el.removeClass('altRow');
			}else{
				el.addClass('altRow');
			}
		});
	},
	
	filter: function(form){
		var form = $(form);
		var col = 0;
		var key = '';
		
		form.getChildren().each(function(el,i){
			if(el.id == 'column'){
				col = Number(el.value);
			}
			if(el.id == 'keyword'){
				key = el.value.toLowerCase();
			}
			if(el.type == 'reset'){
				el.addEvent('click',this.clearFilter.bind(this));
			}
		}, this);
		
		if(key){
		this.elements.each(function(el,i){
			if(this.options.filterHide){
				el.removeClass('altRow');
			}
			if(el.getChildren()[col].firstChild.data.toLowerCase().indexOf(key) > -1){
				el.addClass(this.options.filterSelectedCls);
				if(this.options.filterHide){
					el.removeClass(this.options.filterHideCls);
				}
			}else{
				el.removeClass(this.options.filterSelectedCls);
				if(this.options.filterHide){
					el.addClass(this.options.filterHideCls);
				}
			}
		}, this);
		if(this.options.filterHide){
			this.filteredAltRow();
			this.filtered = true;
		}
		}
	},
	
	clearFilter: function(){
		this.elements.each(function(el,i){
			el.removeClass(this.options.filterSelectedCls);
			if(this.options.filterHide){
				el.removeClass(this.options.filterHideCls);
			}
		}, this);
		if(this.options.filterHide){
			this.altRow();
			this.filtered = false;
		}
	}

});
sortableTable.implement(new Events);
sortableTable.implement(new Options);

/*************************************************************/
