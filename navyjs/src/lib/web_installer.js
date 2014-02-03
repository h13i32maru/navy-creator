/**
 * @typedef {Object} Navy.WebInstaller
 */
Navy.Class.instance('Navy.WebInstaller', {
  _db: null,
  _remoteManifest: null,
  _localManifest: null,
  _invalidResources: null,
  _concurrency: 4,

  initialize: function() {
    this._localManifest = {baseUrl: '', resources: []};
  },

  update: function(manifestUrl, callback) {
    var xhr = new XMLHttpRequest();

    xhr.open('GET', manifestUrl);
    xhr.onload = function(ev){
      var xhr = ev.target;
      this._remoteManifest = JSON.parse(xhr.responseText);

      this._initDB();
    }.bind(this);

    xhr.send();
  },

  _initDB: function() {
    var transaction = function(tr) {
      tr.executeSql('CREATE TABLE IF NOT EXISTS resource (path TEXT PRIMARY KEY, md5 TEXT, content_type TEXT, content TEXT)');
      this._loadLocalManifest();
    }.bind(this);

    function error(e) {
      console.error(e);
    }

    this._db = openDatabase('web_installer', "0.1", "WebInstaller", 5 * 1000 * 1000);
    this._db.transaction(transaction, error);

  },

  _loadLocalManifest: function() {
    var transaction = function(tr) {
      tr.executeSql('SELECT path, md5 from resource', null, function(transaction, result){
        var rows = result.rows;
        for (var i = 0; i < rows.length; i++) {
          console.log(rows.item(0));
        }
        this._pickInvalidResources();
      }.bind(this));
    }.bind(this);

    function error(e) {
      console.error(e);
    }

    this._db.transaction(transaction, error);
  },

  _pickInvalidResources: function() {
    var localResourceMap = this._manifestToResourceMap(this._localManifest);
    var remoteResourceMap = this._manifestToResourceMap(this._remoteManifest);
    var invalidResources = [];

    for (var remotePath in remoteResourceMap) {
      var remoteMD5 = remoteResourceMap[remotePath].md5;

      if (!localResourceMap[remotePath]) {
        invalidResources.push({path: remotePath, md5: remoteMD5});
        continue;
      }

      var localMD5 = localResourceMap[remotePath].md5;
      if (remoteMD5 !== localMD5) {
        invalidResources.push({path: remotePath, md5: remoteMD5});
      }
    }

    this._invalidResources = invalidResources;

    this._startFetchingResources();
  },

  _startFetchingResources: function() {
    for (var i = 0; i < this._concurrency; i++) {
      var loader = new Navy.WebInstaller.Loader();
      loader.onload = this._onFetchResource.bind(this);
      loader.onerror = this._onFetchResourceError.bind(this);
      this._fetchResource(loader);
    }
  },

  _fetchResource: function(loader) {
    if (this._invalidResources.length === 0) {
      return;
    }

    var resource = this._invalidResources.shift();
    loader.load(resource);
  },

  _onFetchResource: function(loader, resource, responseText) {
    console.log(resource, responseText);
    this._fetchResource(loader);
  },

  _onFetchResourceError: function(loader, resource) {
    console.error(resource);
    this._fetchResource(loader);
  },

  _manifestToResourceMap: function(manifest) {
    var resources = manifest.resources;
    var map = {};
    for (var i = 0; i < resources.length; i++) {
      map[resources[i].path] = resources[i];
    }

    return map;
  }

});

/**
 * @class Navy.WebInstaller.Loader
 */
Navy.Class('Navy.WebInstaller.Loader', {
  $static: {
    contentType: {
      '.js': 'text/javascript',
      '.css': 'text/stylesheet',
      '.json': 'text/json',
      '.png': 'image/png',
      '.jpg': 'image/jpg',
      '.jpeg': 'image/jpg',
      '.gif': 'image/gif'
    }
  },

  onload: null,
  onerror: null,

  _resource: null,
  _loaderElement: null,

  load: function(resource) {
    this._resource = resource;
    var path = resource.path;
    var contentType = resource.contentType || this._getContentType(path);

    if (contentType.indexOf('text/') === 0) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', path, true);
      xhr.onload = this._onLoad.bind(this);
      xhr.onerror = this._onError.bind(this);
      xhr.onabort = this._onError.bind(this);
      xhr.send();
      this._loaderElement = xhr;
    } else if (contentType.indexOf('image/') === 0) {
      var image = new Image();
      image.onload = this._onLoad.bind(this);
      image.onerror = this._onError.bind(this);
      image.onabort = this._onError.bind(this);
      image.src = path;
      this._loaderElement = image;
    } else {
      throw new Error('unknown content type. ' + contentType);
    }
  },

  _onLoad: function() {
    if (!this.onload) {
      return;
    }

    if ('responseText' in this._loaderElement) {
      var responseText = this._loaderElement.responseText;
      this.onload(this, this._resource, responseText);
    } else {
      this.onload(this, this._resource);
    }
  },

  _onError: function() {
    if (!this.onerror) {
      return;
    }

    this.onerror(this, this._resource);
  },

  _getContentType: function(path) {
    // TODO クエリパラメータやハッシュが付いている場合に対応.
    var pos = path.lastIndexOf('.');
    var ext = path.substr(pos).toLowerCase();
    var contentType = this.$class.contentType[ext];
    if (!contentType) {
      throw new Error('unknown file extension. ' + ext);
    }

    return contentType;
  }
});