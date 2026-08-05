const viewer = document.querySelector('#helmet-viewer');
const progressValue = document.querySelector('.model-progress__value');
const loadState = document.querySelector('#load-state');
const loadStateLabel = document.querySelector('#load-state-label');
const errorToast = document.querySelector('#error-toast');
const resetButton = document.querySelector('#reset-view');
const rotateButton = document.querySelector('#toggle-rotate');

function setLoadState(label, ready = false) {
  loadStateLabel.textContent = label;
  loadState.classList.toggle('is-ready', ready);
}

viewer.addEventListener('progress', (event) => {
  const progress = Math.max(0, Math.min(1, event.detail.totalProgress));
  progressValue.style.width = `${progress * 100}%`;
  setLoadState(`Loading ${Math.round(progress * 100)}%`);
});

viewer.addEventListener('load', () => {
  progressValue.style.width = '100%';
  setLoadState('Model ready', true);
  window.setTimeout(() => {
    progressValue.style.opacity = '0';
  }, 420);
});

viewer.addEventListener('error', () => {
  setLoadState('Load error');
  errorToast.hidden = false;
});

resetButton.addEventListener('click', () => {
  viewer.cameraTarget = 'auto auto auto';
  viewer.cameraOrbit = '0deg 75deg auto';
  viewer.fieldOfView = '32deg';
  viewer.jumpCameraToGoal();
});

rotateButton.addEventListener('click', () => {
  viewer.autoRotate = !viewer.autoRotate;
  const enabled = viewer.autoRotate;
  rotateButton.textContent = enabled ? 'Auto-rotate on' : 'Auto-rotate off';
  rotateButton.setAttribute('aria-pressed', String(enabled));
});

// Keep the requested defaults explicit even if a browser restores component state.
viewer.autoRotate = true;
viewer.setAttribute('camera-controls', '');
