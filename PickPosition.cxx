#include <vtkActor.h>
#include <vtkPropAssembly.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkPropPicker.h>
#include <vtkWorldPointPicker.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPointHandleRepresentation3D.h>


class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    vtkTypeMacro(MouseInteractorStyle,
        vtkInteractorStyleTrackballCamera);

    virtual void OnLeftButtonDown() override {

        int* clickPos = this->GetInteractor()->GetEventPosition();

        // Pick from this location.
        double * position = this->PickPosition(clickPos);
        if(position != nullptr)
            this->MarkPosition(position);

        // Forward events
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }

    virtual double * PickPosition(const int * clickPosition) = 0;

    void MarkPosition(double * position) {
        auto mark =
            vtkSmartPointer<vtkPointHandleRepresentation3D>::New();
        mark->SetWorldPosition(position);
        auto renderer = this->GetDefaultRenderer();
        mark->SetRenderer(renderer);
        renderer->AddActor(mark);
    }
};


class WorldPointPickerStyle : public MouseInteractorStyle {
public:
    static WorldPointPickerStyle* New();
    vtkTypeMacro(WorldPointPickerStyle,
        MouseInteractorStyle);

    WorldPointPickerStyle() {
        picker = vtkSmartPointer<vtkWorldPointPicker>::New();
    }

    virtual double * PickPosition(const int * clickPos) {
        picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
        return picker->GetPickPosition();
    }

private:
    vtkSmartPointer<vtkWorldPointPicker> picker;
};

vtkStandardNewMacro(WorldPointPickerStyle);

class PropPickerStyle : public MouseInteractorStyle {
public:
    static PropPickerStyle* New();
    vtkTypeMacro(PropPickerStyle,
        MouseInteractorStyle);

    PropPickerStyle() {
        picker = vtkSmartPointer<vtkPropPicker>::New();
    }

    virtual double * PickPosition(const int * clickPos) {
        if(picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer()) == 0)
            return nullptr;
        return picker->GetPickPosition();
    }

private:
    vtkSmartPointer<vtkPropPicker> picker;
};

vtkStandardNewMacro(PropPickerStyle);


class RendererPickStyle : public MouseInteractorStyle {
public:
    static RendererPickStyle* New();
    vtkTypeMacro(RendererPickStyle,
        MouseInteractorStyle);

    RendererPickStyle() {}

    virtual double * PickPosition(const int * clickPos) {
        auto renderer = this->GetDefaultRenderer();
        if(renderer->PickProp(clickPos[0], clickPos[1]) == nullptr)
            return nullptr;

        double display[3];
        display[0] = clickPos[0];
        display[1] = clickPos[1];
        display[2] = renderer->GetPickedZ();
        
        cout<<display[2]<<endl;

        renderer->SetDisplayPoint (display);
        renderer->DisplayToWorld ();
        double * world = renderer->GetWorldPoint ();

        double * picked = new double[3];
        for (int i=0; i < 3; i++) {
            picked[i] = world[i] / world[3];
        }
        
        return picked;
    }
};

vtkStandardNewMacro(RendererPickStyle);


int main(int argc, char* argv[]) {

    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    colors->SetColor("Bkg", 0.3, 0.4, 0.5);

    int numberOfSpheres = 10;
    if (argc > 1) {
        numberOfSpheres = atoi(argv[1]);
    }

    // A renderer and render window
    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(640, 480);
    renderWindow->SetNumberOfLayers(2);

    // add 1st renderer
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetLayer(0);
    renderWindow->AddRenderer(renderer);

    // add a 2nd renderer
    auto renderer1 = vtkSmartPointer<vtkRenderer>::New();
    renderer1->SetLayer(1);
    renderWindow->AddRenderer(renderer1);
    
    // An interactor
    auto renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Set the custom type to use for interaction.
    auto style =
//            vtkSmartPointer<WorldPointPickerStyle>::New();
//            vtkSmartPointer<PropPickerStyle>::New();
        vtkSmartPointer<RendererPickStyle>::New();
    style->SetDefaultRenderer(renderer);

    renderWindowInteractor->SetInteractorStyle(style);

    for (int i = 0; i < numberOfSpheres; ++i) {
        auto source =
                vtkSmartPointer<vtkSphereSource>::New();
        double x, y, z, radius;
        x = vtkMath::Random(-5, 5);
        y = vtkMath::Random(-5, 5);
        z = vtkMath::Random(-5, 5);
        radius = vtkMath::Random(0.5, 1.0);
        source->SetRadius(radius);
        source->SetCenter(x, y, z);
        source->SetPhiResolution(11);
        source->SetThetaResolution(21);
        auto mapper =
                vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(source->GetOutputPort());
        auto actor =
                vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        double r, g, b;
        r = vtkMath::Random(0.4, 1.0);
        g = vtkMath::Random(0.4, 1.0);
        b = vtkMath::Random(0.4, 1.0);
        actor->GetProperty()->SetDiffuseColor(r, g, b);
        actor->GetProperty()->SetDiffuse(0.8);
        actor->GetProperty()->SetSpecular(0.5);
        actor->GetProperty()->SetSpecularColor(
                colors->GetColor3d("White").GetData());
        actor->GetProperty()->SetSpecularPower(30.0);
        renderer->AddActor(actor);
    }

    renderer->SetBackground(colors->GetColor3d("Bkg").GetData());

    // Render and interact
    renderWindow->Render();
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
