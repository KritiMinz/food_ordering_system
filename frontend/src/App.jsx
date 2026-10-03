import { useEffect, useState } from "react"
import Navbar from "./components/Navbar"
import FoodCard from "./components/FoodCard"
import Cart from "./components/Cart"
import FoodDetails from "./components/FoodDetails"
import AuthModal from "./components/AuthModal"
import Checkout from "./components/Checkout"
import Orders from "./components/Orders"
import AdminDashboard from "./components/AdminDashboard"
import Profile from "./components/Profile"

const categories = [
  "All",
  "Burger",
  "Pizza",
  "Pasta",
  "Snacks",
  "Drinks",
]

function App() {
  // ==============================
  // STATE
  // ==============================

  const [cart, setCart] = useState([])

  const [selectedFood, setSelectedFood] = useState(null)

  const [favorites, setFavorites] = useState([])

  const [authMode, setAuthMode] = useState(null)

  const [showCheckout, setShowCheckout] = useState(false)

  const [foods, setFoods] = useState([])

  const [loadingFoods, setLoadingFoods] = useState(true)

  const [searchTerm, setSearchTerm] = useState("")

  const [selectedCategory, setSelectedCategory] = useState("All")

  // Current logged-in user
  const [currentUser, setCurrentUser] = useState(() => {
    const savedUser = localStorage.getItem("currentUser")

    return savedUser
      ? JSON.parse(savedUser)
      : null
  })

  // Successfully placed order
  const [orderSuccess, setOrderSuccess] = useState(null)

  const [showProfile, setShowProfile] = useState(false)
  

  // ==============================
  // LOAD FOOD FROM C++ BACKEND
  // ==============================

  useEffect(() => {
    fetch("http://localhost:18080/api/foods")
      .then((response) => {
        if (!response.ok) {
          throw new Error("Failed to fetch food menu")
        }

        return response.json()
      })
      .then((data) => {
        setFoods(data)
        setLoadingFoods(false)
      })
      .catch((error) => {
        console.error("Failed to load foods:", error)
        setLoadingFoods(false)
      })
  }, [])


  // ==============================
  // PLACE ORDER
  // ==============================

  const handlePlaceOrder = (order) => {
  setOrderSuccess(order)

  setCart([])

  localStorage.removeItem("selectedAddress")

  setShowCheckout(false)
}


  // ==============================
  // LOGOUT
  // ==============================

  const handleLogout = () => {
    localStorage.removeItem("authToken")
    localStorage.removeItem("currentUser")

    setCurrentUser(null)
  }


  // ==============================
  // ADD TO CART
  // ==============================

  const handleAddToCart = (food) => {
    setCart((currentCart) => {
      const existingItem = currentCart.find(
        (item) => item.id === food.id
      )

      if (existingItem) {
        return currentCart.map((item) =>
          item.id === food.id
            ? {
                ...item,
                quantity: item.quantity + 1,
              }
            : item
        )
      }

      return [
        ...currentCart,
        {
          ...food,
          quantity: 1,
        },
      ]
    })
  }


  // ==============================
  // UPDATE CART QUANTITY
  // ==============================

  const handleUpdateQuantity = (foodId, newQuantity) => {
    if (newQuantity <= 0) {
      handleRemoveItem(foodId)
      return
    }

    setCart((currentCart) =>
      currentCart.map((item) =>
        item.id === foodId
          ? {
              ...item,
              quantity: newQuantity,
            }
          : item
      )
    )
  }


  // ==============================
  // REMOVE FROM CART
  // ==============================

  const handleRemoveItem = (foodId) => {
    setCart((currentCart) =>
      currentCart.filter(
        (item) => item.id !== foodId
      )
    )
  }


  // ==============================
  // FAVORITES
  // ==============================

  const handleToggleFavorite = (foodId) => {
    setFavorites((currentFavorites) => {
      if (currentFavorites.includes(foodId)) {
        return currentFavorites.filter(
          (id) => id !== foodId
        )
      }

      return [
        ...currentFavorites,
        foodId,
      ]
    })
  }


  // ==============================
  // CART ITEM COUNT
  // ==============================

  const cartItemCount = cart.reduce(
    (total, item) => total + item.quantity,
    0
  )


  // ==============================
  // SEARCH + CATEGORY FILTER
  // ==============================

  const filteredFoods = foods.filter((food) => {
    const matchesSearch = food.name
      .toLowerCase()
      .includes(searchTerm.toLowerCase())

    const matchesCategory =
      selectedCategory === "All" ||
      food.category === selectedCategory

    return (
      matchesSearch &&
      matchesCategory
    )
  })


  // ==============================
  // PAGE
  // ==============================

  return (
    <div className="min-h-screen bg-gray-100">

      {/* ==============================
          NAVBAR
      ============================== */}

      <Navbar
cartItemCount={cartItemCount}
onLogin={() => setAuthMode("login")}
currentUser={currentUser}
onLogout={handleLogout}
onProfile={() => setShowProfile(true)}
/>


      {currentUser?.role === "admin" ? (
  <AdminDashboard
    currentUser={currentUser}
    onLogout={handleLogout}
  />
) : (

  <>

{/* ==============================
    HERO
============================== */}

<section
  id="home"
  className="bg-orange-50 py-20"
>

  <div className="max-w-7xl mx-auto px-6 text-center">

    <h1 className="text-5xl font-bold text-gray-800 mb-6">
      Delicious Food, Delivered Fast
    </h1>

    <p className="text-lg text-gray-600 mb-8">
      Order your favourite food from the comfort of your home.
    </p>

    {/* SEARCH */}

    <div className="max-w-xl mx-auto">

      <input
        type="text"
        value={searchTerm}
        onChange={(event) =>
          setSearchTerm(event.target.value)
        }
        placeholder="Search for food..."
        className="w-full px-5 py-4 rounded-xl border border-gray-300
                   focus:outline-none focus:ring-2 focus:ring-orange-500"
      />

    </div>

  </div>

</section>

      {/* ==============================
          CATEGORIES
      ============================== */}

      <section className="max-w-7xl mx-auto px-6 py-10">

        <h2 className="text-3xl font-bold text-gray-800 mb-6">
          Categories
        </h2>

        <div className="flex flex-wrap gap-4">

          {categories.map((category) => (

            <button
              key={category}
              onClick={() =>
                setSelectedCategory(category)
              }
              className={
                selectedCategory === category
                  ? "bg-orange-600 text-white px-6 py-3 rounded-full"
                  : "bg-white px-6 py-3 rounded-full shadow hover:bg-orange-50"
              }
            >
              {category}
            </button>

          ))}

        </div>

      </section>


      {/* ==============================
          FOOD MENU
      ============================== */}

      <section
  id="menu"
  className="max-w-7xl mx-auto px-6 pb-16"
>

        <h2 className="text-3xl font-bold text-gray-800 mb-8">
          Popular Food
        </h2>


        {/* LOADING */}

        {loadingFoods && (
          <div className="bg-white rounded-2xl shadow-md p-8 text-center mb-8">

            <p className="text-gray-600">
              Loading food menu...
            </p>

          </div>
        )}


        {/* NO FOOD */}

        {!loadingFoods &&
          filteredFoods.length === 0 && (

            <div className="bg-white rounded-2xl shadow-md p-10 text-center">

              <div className="text-5xl mb-4">
                🔍
              </div>

              <h3 className="text-xl font-bold text-gray-800">
                No food found
              </h3>

              <p className="text-gray-500 mt-2">
                Try another search or category.
              </p>

            </div>

          )}


        {/* FOOD CARDS */}

        {!loadingFoods &&
          filteredFoods.length > 0 && (

            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-8">

              {filteredFoods.map((food) => (

                <FoodCard
                  key={food.id}
                  food={food}
                  onAddToCart={handleAddToCart}
                  onViewDetails={() =>
                    setSelectedFood(food)
                  }
                />

              ))}

            </div>

          )}

      </section>


      {/* ==============================
          CART
      ============================== */}

      <section
  id="cart"
  className="max-w-4xl mx-auto px-6 pb-20"
>

        <Cart
          cart={cart}
          onUpdateQuantity={handleUpdateQuantity}
          onRemoveItem={handleRemoveItem}

          onCheckout={() => {

            // User must login before checkout
            if (!currentUser) {
              setAuthMode("login")
              return
            }

            setShowCheckout(true)
          }}

        />

      </section>

      {/* ==============================
    MY ORDERS
============================== */}

<section id="orders">
  <Orders
    currentUser={currentUser}
  />
</section>

{showProfile && (
  <Profile
    currentUser={currentUser}
    onClose={() => setShowProfile(false)}
    onSelectAddress={(address) => {
  localStorage.setItem("selectedAddress", address)
  setShowProfile(false)
  setShowCheckout(true)
}}
  />
)}
      {/* ==============================
          FOOD DETAILS
      ============================== */}

      <FoodDetails
        food={selectedFood}

        onClose={() =>
          setSelectedFood(null)
        }

        onAddToCart={handleAddToCart}

        isFavorite={
          selectedFood
            ? favorites.includes(
                selectedFood.id
              )
            : false
        }

        onToggleFavorite={
          handleToggleFavorite
        }
      />


      {/* ==============================
          AUTHENTICATION MODAL
      ============================== */}

      {authMode && (

        <AuthModal
          mode={authMode}

          onClose={() =>
            setAuthMode(null)
          }

          onSwitchMode={() =>
            setAuthMode(
              authMode === "login"
                ? "register"
                : "login"
            )
          }

          onLoginSuccess={(user) => {
            setCurrentUser(user)

            localStorage.setItem(
              "currentUser",
              JSON.stringify(user)
            )

            setAuthMode(null)
          }}

        />

      )}


      {/* ==============================
          ORDER CONFIRMATION
      ============================== */}

      {orderSuccess && (

        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50 px-4">

          <div className="bg-white rounded-2xl shadow-xl w-full max-w-md p-8 text-center">

            <div className="text-6xl mb-4">
              🎉
            </div>

            <h2 className="text-3xl font-bold text-gray-800">
              Order Confirmed!
            </h2>

            <p className="text-gray-500 mt-3">
              Your order has been placed successfully.
            </p>


            <div className="bg-orange-50 rounded-lg p-4 mt-6">

              <p className="text-gray-600">
                Order Number
              </p>

              <p className="text-2xl font-bold text-orange-600">
                #{orderSuccess.id}
              </p>

              <p className="text-gray-600 mt-3">
                Total: ₹{orderSuccess.total}
              </p>

              <p className="text-gray-600 mt-2">
                Status: {orderSuccess.status}
              </p>

            </div>


            <button
              onClick={() =>
                setOrderSuccess(null)
              }
              className="w-full bg-orange-600 text-white py-3 rounded-lg
                         font-semibold mt-6 hover:bg-orange-700"
            >
              Continue Shopping
            </button>

          </div>

        </div>

      )}


      {/* ==============================
          CHECKOUT
      ============================== */}

      {showCheckout && (

        <Checkout
  cart={cart}
  currentUser={currentUser}

          onClose={() =>
            setShowCheckout(false)
          }

          onPlaceOrder={
            handlePlaceOrder
          }
        />

      )}
      </>
)}

    </div>
  )}

export default App