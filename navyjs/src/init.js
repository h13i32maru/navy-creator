window.Navy = {};

(function(){
  var scriptElements = document.querySelectorAll('script');
  var selfScriptElement = scriptElements[scriptElements.length - 1];
  if (selfScriptElement.dataset.assetConfig) {
    Navy.assetConfig = JSON.parse(selfScriptElement.dataset.assetConfig);
  } else {
    window.Navy.assetConfig = {alwaysRemote: false, forceUpdate: false};
  }
})();

window.addEventListener('DOMContentLoaded', function(){
  if (Navy.UnitTest) {
    return;
  }

  Navy.AssetInstaller.initialize('./manifest.json');

  // かならずremoteを使う場合は都度サーバから取得することになる.
  Navy.AssetInstaller.setAlwaysRemote(Navy.assetConfig.alwaysRemote);

  Navy.AssetInstaller.update({
    forceUpdate: Navy.assetConfig.forceUpdate,
    onProgress: function(progress, total) {
      var progressElement = document.querySelector('#asset_installer_inner_progress');
      if (progressElement) {
        progressElement.style.width = (100 * progress / total) + '%';
      }
    },
    onComplete: function() {
      var progressElement = document.querySelector('#asset_installer_progress');
      progressElement && progressElement.parentElement.removeChild(progressElement);
      Navy.App.initialize();
    },
    onError: function(path) {
      console.error(path);
    }
  });
});
