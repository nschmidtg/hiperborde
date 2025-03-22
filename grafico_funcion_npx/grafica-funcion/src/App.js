import './App.css';
import FunctionPlotter from './components/FunctionPlotter';
import FunctionPlotterAll from './components/FunctionPlotterAll';

function App() {
  return (
    <div className="App">
      <header className="App-header">
        <h1>Gráfica de la función h·sin²(πx/(C√h))</h1>
        <FunctionPlotterAll />
        <FunctionPlotter />
      </header>
    </div>
    
  );
}

export default App;